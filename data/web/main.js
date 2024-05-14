const map = L.map("map", {
    maxBounds: L.latLngBounds(L.latLng(90, -90), L.latLng(-180, 180)),
    maxBoundsViscosity: 0.8,
}).setView([49, 10], 5);

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    minZoom: 2,
    attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

map.on("click", temp2);

let positions = [];
let routePolyLines = [];

// TODO: investigate and fix distance bug in dijkstra
async function temp(e){
    let nodeLocation = await getNodeLocation(12412);
    L.marker(nodeLocation).addTo(map);
    nodeLocation = await getNodeLocation(4343);
    L.marker(nodeLocation).addTo(map);

    let path = await getShortestPath(12412, 4343)
    if(path.length === 0){
        positions.pop(); // remove last added position because it is invalid
    }else{
        routePolyLines.push(L.polyline(path, {color: "red"}).addTo(map));
    }
}

async function temp2(e){
    let latLng = clampPosition(e.latlng);
    let cellIndex = await getCellIndex(latLng);
    let nodes = await getNodesInCell(cellIndex);
    nodes.forEach((n) => {
        L.marker(n).addTo(map);
    });
}

async function addPosition(e) {
    let latLng = clampPosition(e.latlng);
    let nodeId = await getClosestNodeToPosition(latLng.lat, latLng.lng);
    console.log("Node ID: " + nodeId)
    let nodeLocation = await getNodeLocation(nodeId);
    console.log("Location: ", nodeLocation)

    positions.push(nodeId);
    L.marker(nodeLocation).addTo(map);

    if (positions.length > 1) {
        let path = await getShortestPath(positions[positions.length - 2], positions[positions.length - 1])
        console.log("Path: ",path)
        if(path.length === 0){
            positions.pop(); // remove last added position because it is invalid
        }else{
            routePolyLines.push(L.polyline(path, {color: "red"}).addTo(map));
        }
    }
}

async function getClosestNodeToPosition(latitude, longitude) {
    const res = await fetch("/api/get_node/" + latitude + "/" + longitude);
    const json = await res.json();
    return json["nodeId"];
}

async function getNodeLocation(nodeId) {
    const res = await fetch("/api/get_location/" + nodeId);
    const json = await res.json();
    return L.latLng(json["lat"], json["lon"]);
}

async function getShortestPath(startNodeId, targetNodeId) {
    const res = await fetch("/api/get_path/" + startNodeId + "/" + targetNodeId);
    const json = await res.json();
    const path = [];
    console.log("JSON RES: ",json)

    if (json["distance"] === -1)
        return path; // no path found

    const nodeIds = json["nodes"]
    for (const index in nodeIds) {
        const pos = await getNodeLocation(nodeIds[index]);
        console.log("id: " + nodeIds[index], pos)
        path.push(pos);
    }
    return path;
}



async function getCellIndex(latLng) {
    const res = await fetch("/api/get_cell/" + latLng.lat + "/" + latLng.lng);
    const json = await res.json();
    return json["cellIndex"];
}

async function getNodesInCell(cellIndex) {
    const res = await fetch("/api/get_nodes_in_cell/" + cellIndex);
    const json = await res.json();
    const nodes = [];
    console.log("JSON RES: ",json)

    const nodeIds = json["nodes"]
    for (const index in nodeIds) {
        const pos = await getNodeLocation(nodeIds[index]);
        nodes.push(pos);
    }
    return nodes;
}

function clampPosition(latLng) {
    if (latLng.lng < -180)
        latLng.lng += 360;

    if (latLng.lng >= 180)
        latLng.lng -= 360;

    return latLng
}
