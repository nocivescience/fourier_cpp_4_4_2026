#include <SFML/Graphics.hpp>
#include <complex>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

// Estructura para los coeficientes de Fourier
struct Coefficient {
    int freq;
    double amplitude;
    double phase;
};

// Transformada Discreta de Fourier (DFT)
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
    // Configuración de la ventana
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Pizarra de Fourier - SFML 3");
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

            // Inicio del dibujo
            if (event->is<sf::Event::MouseButtonPressed>()) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    isDrawing = true;
                    playingFourier = false;
                    drawingPoints.clear();
                    fourierPath.clear();
                    fourierCoeffs.clear();
                    time = 0;
                }
            }

            // Al soltar, calculamos la magia matemática
            if (event->is<sf::Event::MouseButtonReleased>()) {
                if (isDrawing && drawingPoints.size() > 5) {
                    isDrawing = false;
                    fourierCoeffs = computeDFT(drawingPoints);
                    
                    // Ordenamos por amplitud: los círculos grandes primero
                    std::sort(fourierCoeffs.begin(), fourierCoeffs.end(), [](const Coefficient& a, const Coefficient& b) {
                        return a.amplitude > b.amplitude;
                    });
                    
                    playingFourier = true;
                } else {
                    isDrawing = false;
                }
            }
        }

        // Captura de puntos del mouse
        if (isDrawing) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            std::complex<double> currentPoint(static_cast<double>(mousePos.x), static_cast<double>(mousePos.y));
            
            if (drawingPoints.empty() || std::abs(currentPoint - drawingPoints.back()) > 3.0) {
                drawingPoints.push_back(currentPoint);
            }
        }

        window.clear(sf::Color(15, 15, 20));

        // MODO DIBUJO: Mostrar lo que el usuario está trazando
        if (isDrawing && drawingPoints.size() > 1) {
            std::vector<sf::Vertex> vertexDrawing;
            for (const auto& p : drawingPoints) {
                vertexDrawing.push_back({{static_cast<float>(p.real()), static_cast<float>(p.imag())}, sf::Color(255, 255, 0, 150)});
            }
            window.draw(vertexDrawing.data(), vertexDrawing.size(), sf::PrimitiveType::LineStrip);
        }

        // MODO REPRODUCCIÓN: Epiciclos de Fourier
        if (playingFourier && !fourierCoeffs.empty()) {
            
            // EL TRUCO: Empezamos en el primer coeficiente (i=0) pero NO lo dibujamos como círculo.
            // Esto elimina el vector gigante que viene desde la esquina (0,0).
            float firstX = static_cast<float>(fourierCoeffs[0].amplitude * std::cos(fourierCoeffs[0].phase));
            float firstY = static_cast<float>(fourierCoeffs[0].amplitude * std::sin(fourierCoeffs[0].phase));
            sf::Vector2f currentPos(firstX, firstY);
            
            // Empezamos el bucle en 1 para saltar el desplazamiento inicial
            for (size_t i = 1; i < fourierCoeffs.size(); i++) {
                const auto& coeff = fourierCoeffs[i];
                sf::Vector2f prevPos = currentPos;
                
                float radius = static_cast<float>(coeff.amplitude);
                float angle = static_cast<float>(coeff.phase) + static_cast<float>(coeff.freq) * time;
                
                currentPos.x += radius * std::cos(angle);
                currentPos.y += radius * std::sin(angle);

                // Dibujar el círculo (órbita) solo si es relevante
                if (radius > 0.5f) {
                    sf::CircleShape circle(radius);
                    circle.setFillColor(sf::Color::Transparent);
                    circle.setOutlineColor(sf::Color(150, 150, 255, 30)); // Azul traslúcido
                    circle.setOutlineThickness(1.0f);
                    circle.setOrigin({radius, radius});
                    circle.setPosition(prevPos);
                    window.draw(circle);

                    // Línea del brazo (radio)
                    sf::Vertex line[] = { 
                        {prevPos, sf::Color(200, 200, 200, 80)}, 
                        {currentPos, sf::Color::White} 
                    };
                    window.draw(line, 2, sf::PrimitiveType::Lines);
                }
            }

            // Guardar y dibujar el rastro de la figura
            fourierPath.push_back(currentPos);
            if (fourierPath.size() > 1) {
                std::vector<sf::Vertex> vPath;
                for (const auto& p : fourierPath) {
                    vPath.push_back({p, sf::Color::Cyan});
                }
                window.draw(vPath.data(), vPath.size(), sf::PrimitiveType::LineStrip);
            }

            // Velocidad de la animación (una vuelta completa basada en los puntos originales)
            time += (2.0f * static_cast<float>(M_PI)) / static_cast<float>(drawingPoints.size());
            
            if (time > 2.0f * M_PI) {
                time = 0;
                fourierPath.clear(); // Reinicia el trazo al completar
            }
        }

        window.display();
    }

    return 0;
}