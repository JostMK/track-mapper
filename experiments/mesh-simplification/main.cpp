//
// Created by Jost on 22/07/2024.
// Modified from https://doc.cgal.org/latest/Surface_mesh_simplification/index.html example 5.9
//

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_filter.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/GarlandHeckbert_policies.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

#include <chrono>
#include <iostream>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Surface_mesh<Point_3> Surface_mesh;

namespace SMS = CGAL::Surface_mesh_simplification;
typedef SMS::GarlandHeckbert_plane_policies<Surface_mesh, Kernel> Classic_plane;
typedef SMS::GarlandHeckbert_probabilistic_plane_policies<Surface_mesh, Kernel> Prob_plane;
typedef SMS::GarlandHeckbert_triangle_policies<Surface_mesh, Kernel> Classic_tri;
typedef SMS::GarlandHeckbert_probabilistic_triangle_policies<Surface_mesh, Kernel> Prob_tri;

template<typename GHPolicies>
void collapse_gh(Surface_mesh &mesh, const double ratio) {
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    const SMS::Edge_count_ratio_stop_predicate<Surface_mesh> stop(ratio);

    // Garland&Heckbert simplification policies + Bounded_normal_change policy
    typedef typename GHPolicies::Get_cost GH_cost;
    typedef typename GHPolicies::Get_placement GH_placement;
    typedef SMS::Bounded_normal_change_placement<GH_placement> Bounded_GH_placement;

    GHPolicies gh_policies(mesh);
    const GH_cost &gh_cost = gh_policies.get_cost();
    const GH_placement &gh_placement = gh_policies.get_placement();
    Bounded_GH_placement placement(gh_placement);

    const int r = SMS::edge_collapse(mesh, stop, CGAL::parameters::get_cost(gh_cost).get_placement(placement));

    const std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::cout << "Time elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
              << std::endl;
    std::cout <<  r << " edges removed, " << mesh.edges().size() << " final edges." << std::endl;
}

void collapse_lt(Surface_mesh &mesh, const double ratio) {
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    const SMS::Edge_count_ratio_stop_predicate<Surface_mesh> stop(ratio);

    // Lindstrom&Turk simplification policies + Bounded_normal_change filter
    typedef SMS::LindstromTurk_placement<Surface_mesh> LT_placement;

    const auto &lt_cost = SMS::LindstromTurk_cost<Surface_mesh>();
    const auto &lt_placement = LT_placement();
    const SMS::Bounded_normal_change_filter<> filter;

    const int r = SMS::edge_collapse(mesh, stop,
                                     CGAL::parameters::get_cost(lt_cost).filter(filter).get_placement(lt_placement));

    const std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::cout << "Time elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << "ms"
              << std::endl;
    std::cout <<  r << " edges removed, " << mesh.edges().size() << " final edges." << std::endl;
}

int main() {
    Surface_mesh mesh;

    std::cout << "Input mesh path: " << std::endl;
    std::string inFilename;
    std::cin >> inFilename;

    std::cout << "Output folder path for meshes: " << std::endl;
    std::string outFoldername;
    std::cin >> outFoldername;

    if (!CGAL::IO::read_polygon_mesh(inFilename, mesh)) {
        std::cerr << "Failed to read input mesh: " << inFilename << std::endl;
        return EXIT_FAILURE;
    }
    if (!CGAL::is_triangle_mesh(mesh)) {
        std::cerr << "Input geometry is not triangulated." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "\nInput mesh has " << num_vertices(mesh) << " vertices, " << num_edges(mesh) << " edges, "
              << num_faces(mesh) << " faces" << std::endl;

    constexpr double ratio = 0.1;

    {
        std::cout << "\nSimplification using <Lindstrom-Turk>:" << std::endl;
        Surface_mesh mesh_copy(mesh);
        collapse_lt(mesh_copy, ratio);
        CGAL::IO::write_polygon_mesh(outFoldername+"LT.obj", mesh_copy, CGAL::parameters::stream_precision(17));
    }

    {
        std::cout << "\nSimplification using <Garland-Heckbert Classic_plane>:" << std::endl;
        Surface_mesh mesh_copy(mesh);
        collapse_gh<Classic_plane>(mesh_copy, ratio);
        CGAL::IO::write_polygon_mesh(outFoldername+"GH-CP.obj", mesh_copy, CGAL::parameters::stream_precision(17));
    }

    {
        std::cout << "\nSimplification using <Garland-Heckbert Classic_triangle>:" << std::endl;
        Surface_mesh mesh_copy(mesh);
        collapse_gh<Classic_tri>(mesh_copy, ratio);
        CGAL::IO::write_polygon_mesh(outFoldername+"GH-CT.obj", mesh_copy, CGAL::parameters::stream_precision(17));
    }

    {
        std::cout << "\nSimplification using <Garland-Heckbert Probabilistic_plane>:" << std::endl;
        Surface_mesh mesh_copy(mesh);
        collapse_gh<Prob_plane>(mesh_copy, ratio);
        CGAL::IO::write_polygon_mesh(outFoldername+"GH-PP.obj", mesh_copy, CGAL::parameters::stream_precision(17));
    }

    {
        std::cout << "\nSimplification using <Garland-Heckbert Probabilistic_triangle>:" << std::endl;
        Surface_mesh mesh_copy(mesh);
        collapse_gh<Prob_tri>(mesh_copy, ratio);
        CGAL::IO::write_polygon_mesh(outFoldername+"GH-PT.obj", mesh_copy, CGAL::parameters::stream_precision(17));
    }

    return EXIT_SUCCESS;
}
