#pragma once

#include <mpi.h>

#include <cstdint>
#include <vector>

#include "../tile.h"

namespace mpi_protocol {

constexpr int kTagWorkRequest = 1;
constexpr int kTagTileAssignment = 2;
constexpr int kTagTileResult = 3;
constexpr int kTagTerminate = 4;

void send_work_request(int destination_rank);
void receive_work_request(int source_rank);

void send_tile_assignment(const Tile& tile, int destination_rank);
Tile receive_tile_assignment(int source_rank);

void send_tile_result(const Tile& tile, const std::vector<uint32_t>& pixels, int destination_rank);
void receive_tile_result(int source_rank, Tile& tile, std::vector<uint32_t>& pixels);

void send_terminate(int destination_rank);
void receive_terminate(int source_rank);

} // namespace mpi_protocol




