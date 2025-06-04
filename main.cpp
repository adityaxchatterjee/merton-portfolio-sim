#include <emscripten.h>
#include "nlohmann/json.hpp"
#include <vector>
#include <cmath>
#include <string>

using namespace std;
using json = nlohmann::json;

extern "C" {

// Safe fallback for division and log/exp
inline double max_safe(double x, double eps = 1e-8) {
    return (std::abs(x) > eps) ? x : (x >= 0 ? eps : -eps);
}

EMSCRIPTEN_KEEPALIVE
const char* solve(double mu, double r, double sigma, double rho, double gamma, double T) {
    static string result_json;  // keep memory alive after return

    int Nw = 100;
    int Nt = 200;
    double w_min = 0.01, w_max = 10.0;
    double dw = (w_max - w_min) / (Nw - 1);
    double dt = T / Nt;

    vector<double> w_grid(Nw), t_grid(Nt + 1);
    for (int i = 0; i < Nw; ++i) {
        w_grid[i] = w_min + i * dw;
    }
    for (int n = 0; n <= Nt; ++n) {
        t_grid[n] = n * dt;
    }
    
    vector<vector<double>> V(Nt + 1, vector<double>(Nw, 0.0));
    vector<vector<double>> pi_star(Nt + 1, vector<double>(Nw, 0.0));
    vector<vector<double>> c_star(Nt + 1, vector<double>(Nw, 0.0));

    
    for (int i = 0; i < Nw; ++i)
        V[Nt][i] = pow(w_grid[i], 1 - gamma) / (1 - gamma);

    for (int n = Nt - 1; n >= 0; --n) {
        for (int i = 1; i < Nw - 1; ++i) {
            double w = max_safe(w_grid[i]);
            double Vw = (V[n+1][i+1] - V[n+1][i-1]) / (2 * dw);
            double Vww = (V[n+1][i+1] - 2 * V[n+1][i] + V[n+1][i-1]) / (dw * dw);

            double Vw_safe = max_safe(Vw);
            double Vww_safe = max_safe(Vww);

            double c = pow(Vw_safe, -1.0 / gamma);
            if (!isfinite(c)) c = 0.0;
            else c = min(c, 1e3);

            double pi = -((mu - r) / (sigma * sigma)) * (Vw_safe / (w * Vww_safe));
            if (!isfinite(pi)) pi = 0.0;
            else pi = std::clamp(pi, -10.0, 10.0);

            double drift = r * w + pi * (mu - r) - c;
            double diffusion = 0.5 * sigma * sigma * pi * pi * w * w;
            double utility = pow(c, 1 - gamma) / (1 - gamma);

            V[n][i] = V[n+1][i] + dt * (drift * Vw + diffusion * Vww + utility - rho * V[n+1][i]);
            if (!isfinite(V[n][i])) V[n][i] = 0.0;

            pi_star[n][i] = pi;
            c_star[n][i] = c;
        }
    }

    
    json output;
    output["wealth"] = w_grid;
    output["time"] = t_grid;
    output["valueFunction"] = V;
    output["piStar"] = pi_star;
    output["cStar"] = c_star;

    result_json = output.dump();
    return result_json.c_str();
}

}