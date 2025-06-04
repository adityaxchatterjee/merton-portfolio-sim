
function transpose(matrix) {
    return matrix[0].map((_, colIndex) => matrix.map(row => row[colIndex]));
}

function plotSurface(divId, data, title, zLabel, dataKey) {
    const matrix = data[dataKey];

    if (!matrix || !Array.isArray(matrix) || matrix.length === 0 || !Array.isArray(matrix[0])) {
        console.error(`Invalid or empty matrix for ${dataKey}:`, matrix);
        return;
    }

    Plotly.newPlot(divId, [{
        x: data.time,
        y: data.wealth,
        z: transpose(matrix),
        type: "surface",
        colorscale: "Viridis"
    }], {
        title,
        scene: {
            xaxis: { title: "Time (t)" },
            yaxis: { title: "Wealth (w)" },
            zaxis: { title: zLabel }
        }
    });
}