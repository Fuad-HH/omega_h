#include <iostream>

#include "Omega_h_cmdline.hpp"
#include "Omega_h_file.hpp"
#include "Omega_h_mesh.hpp"
#include "Omega_h_array_ops.hpp"
#include "Omega_h_for.hpp"

int main(int argc, char** argv) {
  auto lib = Omega_h::Library(&argc, &argv);
  auto world = lib.world();
printf("ok0\n");

  Omega_h::CmdLine cmdline;
  cmdline.add_arg<std::string>("mesh-in");
  cmdline.add_arg<std::string>("model-in");
  cmdline.add_arg<std::string>("mesh-out");
  cmdline.add_arg<int>("nparts_out");

printf("ok1\n");
  auto nparts_total = world->size();
  auto nparts_in = 1;

  if (!cmdline.parse_final(world, &argc, argv)) return -1;
  auto mesh_in = cmdline.get<std::string>("mesh-in");
  auto model_in = cmdline.get<std::string>("model-in");
  auto mesh_out = cmdline.get<std::string>("mesh-out");
  auto nparts_out = cmdline.get<int>("nparts_out");
printf("ok2\n");

  if (nparts_out < 1) {
    if (!world->rank()) {
      std::cout << "error: invalid output part count " << nparts_out << '\n';
    }
    return -1;
  }
  if (nparts_out > nparts_total) {
    if (!world->rank()) {
      std::cout << "error: output part count " << nparts_out
                << " greater than MPI job size " << nparts_total << '\n';
    }
    return -1;
  }
  if (nparts_in > nparts_total) {
    if (!world->rank()) {
      std::cout << "error: input part count " << nparts_in
                << " greater than MPI job size " << nparts_total << '\n';
    }
    return -1;
  }

printf("ok3\n");
  auto is_in = (world->rank() < nparts_in);
  auto comm_in = world->split(int(is_in), 0);
  auto is_out = (world->rank() < nparts_out);
  auto comm_out = world->split(int(is_out), 0);
  auto mesh = Omega_h::Mesh(&lib);
  if (is_in) {
    mesh.set_comm(comm_in);
printf("ok4\n");
    if (nparts_out < nparts_in) {
      Omega_h_fail(
          "partitioning to a smaller part count not yet implemented\n");
    }
  }
printf("ok4.5\n");
  Omega_h::meshsim::matchRead(mesh_in, model_in, comm_in, &mesh, is_in);

printf("ok5 isIn %d isOut %d\n", is_in, is_out);
  if (is_in || is_out) {
    mesh.set_comm(comm_out);
  }
  if (is_out) {
printf("ok5.5\n");
    if (nparts_out != nparts_in) {
      mesh.balance();
    }

printf("ok6 rank= %d\n", world->rank());
    auto rank = world->rank();
    mesh.add_tag<Omega_h::Real>(0, "gravity", 1);
    Omega_h::Write<Omega_h::Real> gravityArray(mesh.nverts(), 0.0, "gravityArray");
    auto leaf_ids = mesh.get_matches(0).leaf_idxs;
    auto root_ids = mesh.get_matches(0).root_idxs;
    auto root_rks = mesh.get_matches(0).root_ranks;
    auto fill_tag = OMEGA_H_LAMBDA (Omega_h::LO i) {
      auto leaf = leaf_ids[i];
      auto root = root_ids[i];
      auto root_rk = root_rks[i];
      if ((root == leaf) && (root_rk == rank)) {
        gravityArray[leaf] = 9.81;
      }
    };
    Omega_h::parallel_for(leaf_ids.size(), fill_tag);
    mesh.set_tag<Omega_h::Real>(0, "gravity", Omega_h::Reals(gravityArray));
    mesh.sync_tag_periodic(0, "gravity");
printf("ok7\n");

    auto tag_out = mesh.get_array<Omega_h::Real>(0, "gravity");
    auto check_tag = OMEGA_H_LAMBDA (Omega_h::LO i) {
      auto leaf = leaf_ids[i];
      OMEGA_H_CHECK((tag_out[leaf] - 9.81) < 0.001);
    };
    Omega_h::parallel_for(leaf_ids.size(), check_tag);
printf("ok8\n");

    Omega_h::binary::write(mesh_out, &mesh);
printf("ok9\n");
  }
printf("ok10\n");

}
