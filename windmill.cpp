#include <SFML/Graphics.hpp>
#include <complex>
#include <vector>
#include <cmath>
#include <algorithm>

struct Coefficient {
    int freq;
    double amplitude;
    double phase;
};

std::vector<Coefficient> computeDFT(const std::vector<std::complex<double>>& x) {
    int N = static_cast<int>(x.size());
    std::vector<Coefficient> X;
    for (int k = 0; k < N; k++) {
        std::complex<double> sum(0, 0);
        for (int n = 0; n < N; n++) {
            double phi = (2.0 * M_PI * k * n) / N;
            sum += x[n] * std::complex<double>(std::cos(phi), -std::sin(phi));
        }
        sum /= static_cast<double>(N);
        X.push_back({k, std::abs(sum), std::arg(sum)});
    }
    return X;
}

int main() {
    // 1. Crear las dos ventanas
    sf::RenderWindow winDraw(sf::VideoMode({600, 600}), "Pizarra: Dibuja aqui");
    sf::RenderWindow winFourier(sf::VideoMode({600, 600}), "Resultado: Serie de Fourier");
    
    winDraw.setFramerateLimit(60);
    winFourier.setFramerateLimit(60);

    std::vector<std::complex<double>> drawingPoints;
    std::vector<Coefficient> fourierCoeffs;
    std::vector<sf::Vector2f> fourierPath;
    
    bool isDrawing = false;
    bool playingFourier = false;
    float time = 0;

    while (winDraw.isOpen() && winFourier.isOpen()) {
        
        // --- EVENTOS VENTANA DE DIBUJO ---
        while (const std::optional event = winDraw.pollEvent()) {
            if (event->is<sf::Event::Closed>()) winDraw.close();

            if (event->is<sf::Event::MouseButtonPressed>()) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    isDrawing = true;
                    playingFourier = false;
                    drawingPoints.clear();
                    fourierPath.clear();
                    time = 0;
                }
            }
            if (event->is<sf::Event::MouseButtonReleased>()) {
                if (isDrawing && drawingPoints.size() > 5) {
                    isDrawing = false;
                    fourierCoeffs = computeDFT(drawingPoints);
                    std::sort(fourierCoeffs.begin(), fourierCoeffs.end(), [](const Coefficient& a, const Coefficient& b) {
                        return a.amplitude > b.amplitude;
                    });
                    playingFourier = true;
                }
            }
        }

        // --- EVENTOS VENTANA DE FOURIER ---
        while (const std::optional event = winFourier.pollEvent()) {
            if (event->is<sf::Event::Closed>()) winFourier.close();
        }

        // --- LOGICA DE CAPTURA ---
        if (isDrawing) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(winDraw);
            std::complex<double> p(mousePos.x, mousePos.y);
            if (drawingPoints.empty() || std::abs(p - drawingPoints.back()) > 2.0) {
                drawingPoints.push_back(p);
            }
        }

        // --- RENDER VENTANA DIBUJO ---
        winDraw.clear(sf::Color(30, 30, 30));
        if (!drawingPoints.empty()) {
            std::vector<sf::Vertex> vDraw;
            for (const auto& p : drawingPoints) 
                vDraw.push_back({{static_cast<float>(p.real()), static_cast<float>(p.imag())}, sf::Color::Yellow});
            winDraw.draw(vDraw.data(), vDraw.size(), sf::PrimitiveType::LineStrip);
        }
        winDraw.display();

        // --- RENDER VENTANA FOURIER ---
        winFourier.clear(sf::Color(15, 15, 25));
        if (playingFourier && !fourierCoeffs.empty()) {
            // Saltamos el componente DC (fourierCoeffs[0]) para centrar
            sf::Vector2f currentPos(static_cast<float>(fourierCoeffs[0].amplitude * std::cos(fourierCoeffs[0].phase)),
                                    static_cast<float>(fourierCoeffs[0].amplitude * std::sin(fourierCoeffs[0].phase)));

            for (size_t i = 1; i < fourierCoeffs.size(); i++) {
                sf::Vector2f prevPos = currentPos;
                float radius = static_cast<float>(fourierCoeffs[i].amplitude);
                float angle = static_cast<float>(fourierCoeffs[i].phase + fourierCoeffs[i].freq * time);
                currentPos.x += radius * std::cos(angle);
                currentPos.y += radius * std::sin(angle);

                sf::CircleShape c(radius);
                c.setFillColor(sf::Color::Transparent);
                c.setOutlineColor(sf::Color(255, 255, 255, 30));
                c.setOutlineThickness(1);
                c.setOrigin({radius, radius});
                c.setPosition(prevPos);
                winFourier.draw(c);
            }
            fourierPath.push_back(currentPos);
            if (fourierPath.size() > 1) {
                std::vector<sf::Vertex> vPath;
                for (const auto& p : fourierPath) vPath.push_back({p, sf::Color::Cyan});
                winFourier.draw(vPath.data(), vPath.size(), sf::PrimitiveType::LineStrip);
            }
            time += (2.0f * M_PI) / drawingPoints.size();
            if (time > 2.0f * M_PI) { time = 0; fourierPath.clear(); }
        }
        winFourier.display();
    }
    return 0;
}