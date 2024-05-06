
const map = L.map("map", {
    maxBounds: L.latLngBounds(L.latLng(90, -90), L.latLng(-180, 180)),
    maxBoundsViscosity: 0.8,
}).setView([0, 0], 2);

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    minZoom: 2,
    attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);