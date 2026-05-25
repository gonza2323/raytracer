#include "mpi_scheduler.h"

#include <algorithm>
#include <iomanip>
#include <chrono>
#include <iostream>
#include <thread>

#include "mpi_protocol.h"

namespace mpi_scheduler {

namespace {

void blit_tile(std::vector<uint32_t>& framebuffer, int framebuffer_width, const Tile& tile,
               const std::vector<uint32_t>& pixels)
{
    int tile_width = tile.x_end - tile.x_start;
    int tile_height = tile.y_end - tile.y_start;

    for (int y = 0; y < tile_height; ++y) {
        const uint32_t* src = pixels.data() + y * tile_width;
        uint32_t* dst = framebuffer.data() + (tile.y_start + y) * framebuffer_width + tile.x_start;
        std::copy(src, src + tile_width, dst);
    }
}

} // namespace

bool run_master_render(Renderer& renderer, int world_size, std::vector<uint32_t>& framebuffer,
                       const MasterCallbacks& callbacks)
{
    int width = renderer.getWidth();
    int height = renderer.getHeight();
    framebuffer.assign(width * height, 0);

    std::vector<Tile> tile_queue = renderer.getTiles();
    int total_tiles = static_cast<int>(tile_queue.size());
    int completed_tiles = 0;
    int active_workers = std::max(0, world_size - 1);

    if (active_workers == 0) {
        std::cout << "[master] no workers available" << std::endl;
        return false;
    }

    while (completed_tiles < total_tiles || active_workers > 0) {
        int has_message = 0;
        MPI_Status status{};
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status);

        if (!has_message) {
            if (callbacks.on_idle && !callbacks.on_idle()) {
                for (int rank = 1; rank < world_size; ++rank) {
                    mpi_protocol::send_terminate(rank);
                }
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        int source = status.MPI_SOURCE;
        if (status.MPI_TAG == mpi_protocol::kTagWorkRequest) {
            mpi_protocol::receive_work_request(source);
            if (!tile_queue.empty()) {
                Tile tile = tile_queue.back();
                tile_queue.pop_back();
                mpi_protocol::send_tile_assignment(tile, source);
            } else {
                mpi_protocol::send_terminate(source);
                active_workers--;
            }
        } else if (status.MPI_TAG == mpi_protocol::kTagTileResult) {
            Tile tile{};
            std::vector<uint32_t> pixels;
            mpi_protocol::receive_tile_result(source, tile, pixels);
            blit_tile(framebuffer, width, tile, pixels);
            completed_tiles++;

            if (callbacks.on_tile) {
                callbacks.on_tile(framebuffer, completed_tiles, total_tiles);
            }

            int percent = (completed_tiles * 100) / total_tiles;
            std::cout << "\rProgress: " << std::setw(3) << percent << "%" << std::flush;
            if (completed_tiles == total_tiles) {
                std::cout << "\nRendering complete!" << std::endl;
            }
        }
    }

    return true;
}

void run_worker_render(Renderer& renderer)
{
    while (true) {
        mpi_protocol::send_work_request(0);

        MPI_Status status{};
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == mpi_protocol::kTagTerminate) {
            mpi_protocol::receive_terminate(0);
            return;
        }

        if (status.MPI_TAG == mpi_protocol::kTagTileAssignment) {
            Tile tile = mpi_protocol::receive_tile_assignment(0);
            std::vector<uint32_t> pixels;
            renderer.render_tile(tile, pixels);
            mpi_protocol::send_tile_result(tile, pixels, 0);
            continue;
        }
    }
}

} // namespace mpi_scheduler




