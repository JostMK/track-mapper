# Track Mapper
A tool for kick-starting the process of making [Assetto Corsa](https://assettocorsa.gg/assetto-corsa/) 
maps based on real-world data.

## Manual flow
1. Create terrain mesh[^terrain_mesh]
2. Trace road as path
3. Project path onto terrain
4. Extend grass mesh along path[^grass_mesh]
5. Subtract terrain mesh by grass mesh
6. Project outer part of grass mesh onto terrain
7. Extend road mesh along path[^road_mesh]
8. Fix self intersecting mesh
9. Split meshes into acceptable sizes
10. UV unwrap all meshes
11. Name every mesh accordingly 
12. Export fbx file

## App flow
1. User starts app
2. App starts web-interface
3. User selects path from OSM in web-interface
4. User uploads geo raster graphics in web-interface
5. App creates terrain meshes[^terrain_mesh]
6. App simplifies terrain mesh based on road proximity
7. App merges all terrain meshes
8. App creates grass mesh[^grass_mesh]
9. App subtracts terrain mesh by grass mesh
10. App projects grass mesh edge onto terrain mesh
11. App creates road mesh without self intersections[^road_mesh][^research_needed]
12. App adds artificial detail to road mesh[^research_needed]
13. App splits meshes into acceptable sizes
14. App assigns UVs[^research_needed]
15. App names meshes accordingly
16. App exports fbx file

## Todo
- [ ] Data stage
  - [ ] Webserver
    - [x] Graph for roads
    - [x] Dijkstra pathfinding for connecting waypoints
    - [x] Create [Crow](https://crowcpp.org/master/) webserver
    - [x] Front-end
    - [x] FIX: cmake setup copying web files
    - [ ] Start crow in extra thread
    - [ ] Allow geo raster uploads
    - [ ] Display geo raster outline on map[^research_needed]
  - [ ] App
    - [ ] Starts webserver
    - [ ] Opens website in browser
    - [ ] Stores all uploaded data (paths, geo files)
- [ ] Mesh stage
  - [ ] Terrain mesh generation[^terrain_mesh]
    - [x] Read in raster data
    - [x] Create terrain meshes
    - [ ] Create heat maps for detail reduction based on road proximity[^research_needed]
    - [ ] Simplify terrain meshes based on heat maps
    - [ ] Merge terrain meshes
    - [ ] Multi-thread for each raster
    - [ ] Assign UVs[^research_needed]
  - [ ] Road generation
    - [ ] Project path onto terrain
      - [ ] Find good sample distance[^research_needed]
    - [ ] Grass mesh generation[^grass_mesh]
      - [ ] Creates mesh along path without self intersections[^research_needed]
      - [ ] Subtract grass mesh from terrain mesh
      - [ ] Project edge of grass mesh onto terrain
      - [ ] Assign UVs[^research_needed]
    - [ ] Road mesh generation[^road_mesh]
      - [ ] Creates mesh along path without self intersections[^research_needed]
      - [ ] Add artificial detail to road
      - [ ] Assign UVs[^research_needed]
  - [ ] Exporting
    - [ ] Split meshes two conform size restrictions
    - [ ] Name meshes following KsEditor naming convention
    - [ ] Export as FBX file[^research_needed]

[^terrain_mesh]: Mesh without collision only for visuals in the background, based on geo data

[^grass_mesh]: Mesh with collision directly next to the road, low detail

[^road_mesh]: Mesh with collision representing the road, very detailed

[^research_needed]: This feature still needs some research into feasibility
