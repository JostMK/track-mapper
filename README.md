# Track Mapper
A tool for kick-starting the process of making [Assetto Corsa](https://assettocorsa.gg/assetto-corsa/) 
maps based on real-world data.

## Building

Following packages are needed:
- [GDAL](https://gdal.org/en/latest/index.html)
- [PROJ](https://proj.org/en/9.5/) *[should be included in GDAL]*
- [CGAL](https://www.cgal.org/)
- [Crow](https://crowcpp.org/master/)
- [FBX SDK](https://aps.autodesk.com/developer/overview/fbx-sdk)

After installing them, configure cmake in the root directory and build the ``TrackMapperServerWebApp`` target.

> [!IMPORTANT]
> Make sure to set the cmake variable ``FBX_ROOT="path/to/fbx/sdk"`` when compiling ([details](./src/scene/README.md)).

> [!TIP]
> After compiling make sure the ``static`` folder containing the [web files](./src/web/static) got copied right next to the executable.
As well as the ``proj`` folder from the *PROJ* library containing runtime assets used by the library.

> [!NOTE] 
> *Cmake should do this automatically however I am far from an expert in cmake so things sometimes don't work as intended*

## Running

After building the project one can start the app by launching the compiled executable.

The app will prompt for the path to a ``.fmi`` file.
Those can be created using the [OsmGraphCreator](https://github.com/fmi-alg/OsmGraphCreator) on ``.osm.pbf`` files, that can be obtained from [Geofabrik](https://download.geofabrik.de/).

> [!TIP]
> Clicking on the name of a region on the [Geofabrik](https://download.geofabrik.de/) website shows all the subregions. This allows to only download files for specific local regions, which reduces the file size significantly.

After loading the supplied graph file, a website will be launched in the system defined default browser.
This is the web interface for creating tracks.

> [!TIP] 
> In case an error, with a message similar to 'missing projection reference' appears, while adding a raster file.
> Then manually add a projection reference using the [ogc wkt](https://www.ogc.org/standard/wkt-crs/) format.
> Those can be obtained for example from [EPSG.org](https://epsg.org/search/by-name).

## Structure

- [data](./data) - For storing example data files
- [experiments](./experiments) - Contains small projects each dedicated to exploring one topic
- [src](./src) - Contains the modules of the project
  - [app](./src/app) - A console application for creating simple tracks
  - [graph](./src/graph) - Holds functionality for node querying and pathfinding on road graphs
  - [mesh](./src/mesh) - Holds functionality for creating and modifying meshes
  - [scene](./src/scene) - Holds functionality for creating and exporting tracks as fbx files
  - [web](./src/web) - Holds functionality for deploying a web interface
    - [static](./src/web/static) - Contains the static files for the web interface


