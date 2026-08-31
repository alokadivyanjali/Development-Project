function generateAIRoute() {
    let toVisit = []; // Array of bins needing collection

    // 1. Filter out safe bins. Only target full or vandalized bins.
    Object.keys(binLocations).forEach(id => {
        let data = binLocations[id].data;
        if (parseInt(data.fill) >= 80 || data.tilt === "TILTED") {
            toVisit.push({ id: id, lat: binLocations[id].lat, lng: binLocations[id].lng });
        }
    });

    let currentLoc = { lat: BASE_LAT, lng: BASE_LNG, id: "HQ Base" };
    let routeSteps = ["HQ Base"];
    let totalDist = 0;

    // 2. Nearest Neighbor Loop
    while(toVisit.length > 0) {
        let nearestIdx = 0; 
        let minDist = Infinity;
        
        // Find the absolute closest bin to the current location
        for(let i=0; i < toVisit.length; i++) {
            let d = parseFloat(calcDistance(currentLoc.lat, currentLoc.lng, toVisit[i].lat, toVisit[i].lng));
            if(d < minDist) { 
                minDist = d; 
                nearestIdx = i; 
            }
        }
        
        // Add closest bin to route, add distance, and remove from pending list
        totalDist += minDist;
        currentLoc = toVisit[nearestIdx];
        routeSteps.push(currentLoc.id);
        toVisit.splice(nearestIdx, 1);
    }
    
    // 3. Return to HQ
    totalDist += parseFloat(calcDistance(currentLoc.lat, currentLoc.lng, BASE_LAT, BASE_LNG));
    routeSteps.push("Return to HQ");
    
    // ... UI rendering logic ...
}
