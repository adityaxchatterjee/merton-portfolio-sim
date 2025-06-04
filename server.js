const express = require("express");
const { spawn } = require("child_process");
const path = require("path");
const app = express();
const PORT = 3000; 

app.use(express.static("public"));

app.get("/run-simulation", (req, res) => {
    const { mu, r, sigma, rho, gamma, T} = req.query;
    const params = [mu, r, sigma, rho, gamma, T].map(parseFloat);
    if (params.some((v) => isNan(v))) {
        console.error("Invalid parameters", req.query);
        return res.status(400).send("Invalid numerical inputs");
    }

    const hjbProcess = spawn("./hjb_solver.exe", params.map(String));

    hjbProcess.stdout.on("data", (data) => {
        console.log(`Simulation output: \n${data.toString()}`);
    });

    hjbProcess.on("close", (code) => {
        if (code === 0) {
            console.log("Simulation active");
            res.status(200).send("Simulation active");
        } else {
            console.error('Simulation failed: Exited with code ${code');
            res.status(500).send("Simulation failed");
        }
    });
});

app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});