const pathEntryList = document.getElementById('path-list');
const pathEntryPlaceholder = document.getElementById('path-placeholder');
const pathEntryTemplate = document.getElementById('path-template');
const pathClosedCheckbox = document.getElementById('input-track-closed');
const pathClosedVisual = document.getElementById('path-closed-checkbox');
pathClosedVisual.style.visibility = 'hidden';
pathClosedCheckbox.addEventListener('change', onClosedToggles)

const CLICK_MODE_NONE = 'click-none';
const CLICK_MODE_PATH = 'click-path';
let clickMode = CLICK_MODE_NONE;

let curPath; // temporarily hold all the data while path is created by user
let paths = {}; // stores all paths for giving to the server later
let pathCounter = 0; // strictly increasing index / id for created paths

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

const pathPathAddBtn = document.getElementById('path-add-btn');
pathPathAddBtn.addEventListener("click", () => {
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

    pathPathAddBtn.value = "Finish";
    pathPathAddBtn.classList.add('btn-pending');

    pathClosedCheckbox.checked = false;
    pathClosedVisual.style.visibility = 'visible';
}

async function finishAddingPath() {
    clickMode = CLICK_MODE_NONE;

    // handle visuals
    pathPathAddBtn.value = "Add Path";
    pathPathAddBtn.classList.remove('btn-pending');

    pathClosedVisual.style.visibility = 'hidden';

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