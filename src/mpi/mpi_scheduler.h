#pragma once

#include <functional>
#include <vector>

#include "../renderer.h"

namespace mpi_scheduler {

struct MasterCallbacks {
	std::function<void(const std::vector<uint32_t>& framebuffer, int completed, int total)> on_tile;
	std::function<bool()> on_idle;
};

bool run_master_render(Renderer& renderer, int world_size, std::vector<uint32_t>& framebuffer,
					   const MasterCallbacks& callbacks);
void run_worker_render(Renderer& renderer);

} // namespace mpi_scheduler





