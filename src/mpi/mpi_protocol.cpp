#include "mpi_protocol.h"

namespace mpi_protocol {

void send_work_request(int destination_rank)
{
    MPI_Send(nullptr, 0, MPI_INT, destination_rank, kTagWorkRequest, MPI_COMM_WORLD);
}

void receive_work_request(int source_rank)
{
    MPI_Recv(nullptr, 0, MPI_INT, source_rank, kTagWorkRequest, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void send_tile_assignment(const Tile& tile, int destination_rank)
{
    int payload[4] = {tile.x_start, tile.x_end, tile.y_start, tile.y_end};
    MPI_Send(payload, 4, MPI_INT, destination_rank, kTagTileAssignment, MPI_COMM_WORLD);
}

Tile receive_tile_assignment(int source_rank)
{
    int payload[4] = {0, 0, 0, 0};
    MPI_Recv(payload, 4, MPI_INT, source_rank, kTagTileAssignment, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return Tile{payload[0], payload[1], payload[2], payload[3]};
}

void send_tile_result(const Tile& tile, const std::vector<uint32_t>& pixels, int destination_rank)
{
    int header[4] = {tile.x_start, tile.x_end, tile.y_start, tile.y_end};
    MPI_Send(header, 4, MPI_INT, destination_rank, kTagTileResult, MPI_COMM_WORLD);
    MPI_Send(pixels.data(), static_cast<int>(pixels.size()), MPI_UINT32_T, destination_rank, kTagTileResult, MPI_COMM_WORLD);
}

void receive_tile_result(int source_rank, Tile& tile, std::vector<uint32_t>& pixels)
{
    int header[4] = {0, 0, 0, 0};
    MPI_Recv(header, 4, MPI_INT, source_rank, kTagTileResult, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    tile = Tile{header[0], header[1], header[2], header[3]};

    int pixel_count = (tile.x_end - tile.x_start) * (tile.y_end - tile.y_start);
    pixels.resize(pixel_count);
    MPI_Recv(pixels.data(), pixel_count, MPI_UINT32_T, source_rank, kTagTileResult, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void send_terminate(int destination_rank)
{
    MPI_Send(nullptr, 0, MPI_INT, destination_rank, kTagTerminate, MPI_COMM_WORLD);
}

void receive_terminate(int source_rank)
{
    MPI_Recv(nullptr, 0, MPI_INT, source_rank, kTagTerminate, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

} // namespace mpi_protocol


