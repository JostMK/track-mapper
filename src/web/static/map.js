const map = L.map("map", {
    maxBounds: L.latLngBounds(L.latLng(90, -90), L.latLng(-180, 180)),
    maxBoundsViscosity: 0.8,
}).setView([49, 10], 5);

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    minZoom: 2,
    attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

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

    if (json["distance"] === -1)
        return path; // no path found

    json["nodes"].forEach((node) => {
        path.push(L.latLng(node["lat"], node["lon"]));
    });

    return path;
}

async function getRasterExtend(rasterFilePath) {
    const res = await fetch("/api/get_raster_extend/" + btoa(rasterFilePath.replaceAll('\\', '/')));
    const json = await res.json();
    const rect = [];

    if(json["error"] != undefined){
        console.error(json["error"])
        return null;
    }

    json["corners"].forEach((node) => {
        rect.push(L.latLng(node["lat"], node["lon"]));
    });

    return rect;
}

function clampPosition(latLng) {
    // map is clamped to bounds so no offset bigger can happen
    if (latLng.lng < -180)
        latLng.lng += 360;

    if (latLng.lng >= 180)
        latLng.lng -= 360;

    return latLng
}
