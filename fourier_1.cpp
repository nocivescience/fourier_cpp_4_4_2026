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

// Transformada Discreta de Fourier
std::vector<Coefficient> computeDFT(const std::vector<std::complex<double>>& x) {
    int N = static_cast<int>(x.size());
    std::vector<Coefficient> X;
    X.reserve(N);
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
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Dibuja algo y Fourier lo trazara");
    window.setFramerateLimit(60);

    std::vector<std::complex<double>> drawingPoints;
    std::vector<Coefficient> fourierCoeffs;
    std::vector<sf::Vector2f> fourierPath;
    
    bool isDrawing = false;
    bool playingFourier = false;
    float time = 0;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Detectar inicio de dibujo
            if (event->is<sf::Event::MouseButtonPressed>()) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    isDrawing = true;
                    playingFourier = false;
                    drawingPoints.clear();
                    fourierPath.clear();
                    time = 0;
                }
            }

            // Detectar fin de dibujo y calcular Fourier
            if (event->is<sf::Event::MouseButtonReleased>()) {
                if (isDrawing && drawingPoints.size() > 2) {
                    isDrawing = false;
                    fourierCoeffs = computeDFT(drawingPoints);
                    // Ordenar por amplitud para mejores epiciclos
                    std::sort(fourierCoeffs.begin(), fourierCoeffs.end(), [](const Coefficient& a, const Coefficient& b) {
                        return a.amplitude > b.amplitude;
                    });
                    playingFourier = true;
                }
            }
        }

        // Lógica de captura de puntos
        if (isDrawing) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            std::complex<double> currentPoint(mousePos.x, mousePos.y);
            
            // Solo añadir si el mouse se movió un poco para no saturar
            if (drawingPoints.empty() || std::abs(currentPoint - drawingPoints.back()) > 2.0) {
                drawingPoints.push_back(currentPoint);
            }
        }

        window.clear(sf::Color(20, 20, 25));

        // RENDERIZADO MODO DIBUJO
        if (isDrawing) {
            std::vector<sf::Vertex> vertexDrawing;
            for (const auto& p : drawingPoints) {
                vertexDrawing.push_back({{static_cast<float>(p.real()), static_cast<float>(p.imag())}, sf::Color::Yellow});
            }
            window.draw(vertexDrawing.data(), vertexDrawing.size(), sf::PrimitiveType::LineStrip);
        }

        // RENDERIZADO MODO FOURIER
        if (playingFourier) {
            sf::Vector2f currentPos(0, 0); // La DFT ya contiene la posición absoluta
            
            for (const auto& coeff : fourierCoeffs) {
                sf::Vector2f prevPos = currentPos;
                float radius = static_cast<float>(coeff.amplitude);
                float angle = static_cast<float>(coeff.phase) + static_cast<float>(coeff.freq) * time;
                
                currentPos.x += radius * std::cos(angle);
                currentPos.y += radius * std::sin(angle);

                // No dibujamos el primer "círculo" gigante que es solo el offset
                if (coeff.freq != 0 || radius < 1000) { 
                    sf::CircleShape circle(radius);
                    circle.setFillColor(sf::Color::Transparent);
                    circle.setOutlineColor(sf::Color(100, 100, 100, 40));
                    circle.setOutlineThickness(1.0f);
                    circle.setOrigin({radius, radius});
                    circle.setPosition(prevPos);
                    window.draw(circle);
                }

                sf::Vertex line[] = { {prevPos, sf::Color(200, 200, 200, 100)}, {currentPos, sf::Color::White} };
                window.draw(line, 2, sf::PrimitiveType::Lines);
            }

            fourierPath.push_back(currentPos);
            if (fourierPath.size() > 1) {
                std::vector<sf::Vertex> vPath;
                for (const auto& p : fourierPath) vPath.push_back({p, sf::Color::Cyan});
                window.draw(vPath.data(), vPath.size(), sf::PrimitiveType::LineStrip);
            }

            // Velocidad de avance (ajustada al número de puntos)
            time += (2.0f * static_cast<float>(M_PI)) / static_cast<float>(drawingPoints.size());
            if (time > 2.0f * M_PI) {
                time = 0;
                fourierPath.clear();
            }
        }

        window.display();
    }

    return 0;
}