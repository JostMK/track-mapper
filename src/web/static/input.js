// -- Adding Paths Variables --
const pathEntryList = document.getElementById('path-list');
const pathEntryPlaceholder = document.getElementById('path-placeholder');
const pathEntryTemplate = document.getElementById('path-template');
const pathAddBtn = document.getElementById('path-add-btn');
const pathClosedCheckbox = document.getElementById('input-track-closed');
const pathClosedVisual = document.getElementById('path-closed-checkbox');
pathClosedVisual.classList.add('hide');
pathClosedCheckbox.addEventListener('change', onClosedToggles)

const CLICK_MODE_NONE = 'click-none';
const CLICK_MODE_PATH = 'click-path';
let clickMode = CLICK_MODE_NONE;

let curPath; // temporarily hold all the data while path is created by user
let paths = {}; // stores all paths for giving to the server later
let pathCounter = 0; // strictly increasing index / id for created paths

// -- Adding Rasters Variables --
const rasterEntryList = document.getElementById('raster-list');
const rasterEntryPlaceholder = document.getElementById('raster-placeholder');
const rasterEntryTemplate = document.getElementById('raster-template');
const rasterAddBtn = document.getElementById('raster-add-btn');
const rasterCustomProjRef = document.getElementById('input-proj-ref');
const rasterPopupForm = document.getElementById('popup-raster-form');
const rasterPopupFilepath = document.getElementById('popup-raster-path');
rasterPopupForm.classList.add('hide');

let rasters = {}; // stores all raster paths for giving to the server later
let rasterCounter = 0; // strictly increasing index / id for created rasters

// -- Creating Track Variables --
const trackName = document.getElementById('input-track-name');
const outputPath = document.getElementById('input-output-path');
const progressText = document.getElementById('progress-text');
const progressSpinner = document.getElementById('progress-spinner');
const progressPopup = document.getElementById('progress-popup');
progressPopup.classList.add('hide');

const PROGRESS_UPDATE_INTERVAL_MS = 2000;
let progressUpdaterId;

// -- Adding Paths Functionality --
map.on("click", onMapClick);

async function onMapClick(click) {
    if (clickMode === CLICK_MODE_NONE)
        return;

    if (clickMode === CLICK_MODE_PATH) {
        addSegment(click);
    }
}

function onClosedToggles(_) {
    if (clickMode === CLICK_MODE_NONE)
        return;

    if (clickMode === CLICK_MODE_PATH) {
        curPath.isClosed = pathClosedCheckbox.checked;
    }
}

pathAddBtn.addEventListener("click", () => {
    if (clickMode === CLICK_MODE_NONE) startAddingPath();
    else finishAddingPath();
});

function startAddingPath() {
    clickMode = CLICK_MODE_PATH;

    curPath = {
        positions: [],
        markers: [],
        segments: [],
        isClosed: false
    };

    pathAddBtn.value = "Finish";
    pathAddBtn.classList.add('btn-pending');

    pathClosedCheckbox.checked = false;
    pathClosedVisual.classList.remove('hide');
}

async function finishAddingPath() {
    clickMode = CLICK_MODE_NONE;

    // handle visuals
    pathAddBtn.value = "Add Path";
    pathAddBtn.classList.remove('btn-pending');

    pathClosedVisual.classList.add('hide');

    // always delete all markers from the map
    curPath.markers.forEach(m => { map.removeLayer(m); });
    delete curPath.markers;

    // if not enough points where added skip this path
    if (curPath.positions.length < 2)
        return;

    // if this is the first added path in the list hide the placeholder text
    if (Object.keys(paths).length <= 0)
        pathEntryPlaceholder.style.display = "none";

    // if is closed got set add the final segment to the path
    if (curPath.isClosed) {
        curPath.positions.push(curPath.positions[0]);
        const segment = await getShortestPath(curPath.positions.at(-2), curPath.positions.at(-1));
        console.log("Closing Segment: ", segment);

        const poly = L.polyline(segment, { color: "red" });
        map.addLayer(poly);
        curPath.segments.push(poly);
    }

    // add path to the paths dictionary using the strictly increasing pathCounter as key
    const pathIndex = pathCounter++;
    paths[pathIndex] = curPath;

    // clone path entry template and set it up
    const pathEntry = pathEntryTemplate.cloneNode(true);
    pathEntry.innerHTML = pathEntry.innerHTML.replaceAll("::path-name::", "Path " + pathIndex);
    pathEntry.classList.remove("template");
    pathEntry.id = "path-" + pathIndex;

    // setup delete button of path entry
    const deleteBtn = pathEntry.querySelector(".btn-delete");
    deleteBtn.addEventListener("click", () => {
        const path = paths[pathIndex];
        path.segments.forEach(s => { map.removeLayer(s); });
        document.getElementById("path-" + pathIndex).remove();

        delete paths[pathIndex];
        if (Object.keys(paths).length === 0) // if no more paths are left reshow the placeholder text
            pathEntryPlaceholder.style.display = "list-item";
    });

    pathEntryList.appendChild(pathEntry);
}

async function addSegment(click) {
    const latLng = clampPosition(click.latlng);

    const nodeId = await getClosestNodeToPosition(latLng.lat, latLng.lng);
    console.log("Node ID: " + nodeId);

    if (nodeId === -1) {
        alert("No nearby node found in clicked area");
        return;
    }

    const nodeLocation = await getNodeLocation(nodeId);
    console.log("Location: ", nodeLocation);

    curPath.positions.push(nodeId);
    const marker = L.marker(nodeLocation);
    map.addLayer(marker);
    curPath.markers.push(marker);

    if (curPath.positions.length > 1) {
        const segment = await getShortestPath(curPath.positions.at(-2), curPath.positions.at(-1));
        console.log("Segment: ", segment);

        if (segment.length === 0) {
            curPath.positions.pop();
            const marker = curPath.markers.pop();
            map.removeLayer(marker);
            alert("No path fround to last position");
            return;
        }

        const poly = L.polyline(segment, { color: "red" });
        map.addLayer(poly);
        curPath.segments.push(poly);
    }
}

// -- Adding Rasters Functionality --
rasterAddBtn.addEventListener("click", openRasterPopup);

function openRasterPopup() {
    rasterPopupForm.classList.remove('hide');
    rasterPopupFilepath.value = "";
}

function closeRasterPopup() {
    rasterPopupForm.classList.add('hide');
}

async function addRaster() {
    closeRasterPopup();

    // replace '\' with '/' for base64 encoding
    const filePath = rasterPopupFilepath.value.replaceAll('\\', '/');
    const req = {
        filePath: filePath
    };

    // check if custom proj ref is provided
    const projRef = rasterCustomProjRef.value.trim();
    if (projRef !== "") {
        req["projRef"] = projRef;
    }

    // gets extend of raster and draws it on the map
    const extend = await getRasterExtend(JSON.stringify(req));
    if (extend instanceof String || typeof extend === 'string') {
        alert("Error while trying to open raster:\n" + extend);
        return;
    }

    // if this is the first added path in the list hide the placeholder text
    if (Object.keys(rasters).length <= 0)
        rasterEntryPlaceholder.style.display = "none";

    const poly = L.polyline([extend[0], extend[1], extend[3], extend[2], extend[0]], { color: "green" });
    map.addLayer(poly);

    // creates new raster object
    const newRaster = {
        filePath: filePath,
        polyline: poly
    };

    // add raster to the rasters dictionary using the strictly increasing rasterCounter as key
    const rasterIndex = rasterCounter++;
    rasters[rasterIndex] = newRaster;

    // clone raster entry template and set it up
    const rasterEntry = rasterEntryTemplate.cloneNode(true);
    rasterEntry.innerHTML = rasterEntry.innerHTML.replaceAll("::raster-name::", "Raster " + rasterIndex);
    rasterEntry.classList.remove("template");
    rasterEntry.id = "raster-" + rasterIndex;

    // setup delete button of raster entry
    const deleteBtn = rasterEntry.querySelector(".btn-delete");
    deleteBtn.addEventListener("click", () => {
        const raster = rasters[rasterIndex];
        map.removeLayer(raster.polyline);
        document.getElementById("raster-" + rasterIndex).remove();

        delete rasters[rasterIndex];
        if (Object.keys(rasters).length === 0) // if no more rasters are left reshow the placeholder text
            rasterEntryPlaceholder.style.display = "list-item";
    });

    rasterEntryList.appendChild(rasterEntry);

}

// -- Adding Track Creation --
async function startTrackCreation(){
    // show progress popup
    progressText.innerText = "Submitting Track Data";
    progressPopup.classList.remove('hide');

    // create track object
    let name = trackName.value.trim();
    if (name === "")
        name = "TestTrack";

    const outputLocation = outputPath.value.trim();

    // > collect the file path for all rasters
    const rasterFilePaths = []
    Object.keys(rasters).forEach(r => rasterFilePaths.push(rasters[r].filePath));

    // > collect all the points for each path
    const pointsOfPaths = []
    Object.keys(paths).forEach(p => pointsOfPaths.push(paths[p].positions));

    const projRef = rasterCustomProjRef.value.trim();

    const track = {
        name: name,
        rasters: rasterFilePaths,
        paths: pointsOfPaths,
        output: outputLocation,
        wkt: projRef
    }

    // send track for creation to backend
    const reqJson = JSON.stringify(track);
    console.log("Sending create track payload:", reqJson);
    console.log(track);
    const res = await fetch("/api/create_track/" + btoa(reqJson));

    // check if data send is valid
    const json = await res.json();
    if (json["error"]) {
        console.error(json["error"]);
        alert("Error while trying to create track:\n" + json["error"]);
        return;
    }

    // start periodic progress checker
    progressUpdaterId = setInterval(updateProgress, PROGRESS_UPDATE_INTERVAL_MS);
    updateProgress();
}

async function updateProgress(){
    const res = await fetch("/api/get_progress");
    const json = await res.json();

    progressText.innerText = json["progress"];

    if(json["finished"]){
        console.log("Track creation finished!");
        clearInterval(progressUpdaterId);
        progressText.innerText = "Track creation finished, tab can be closed now..";
        progressSpinner.classList.add('hide');
    }

    if (json["error"]) {
        console.log("Track creation failed!");
        console.error(json["error"]);
        alert("Error while trying to create track:\n" + json["error"]);

        clearInterval(progressUpdaterId);
        progressPopup.classList.add('hide');
        return;
    }
}
