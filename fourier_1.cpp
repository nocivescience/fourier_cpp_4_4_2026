#include <SFML/Graphics.hpp>
#include <complex>
#include <vector>
#include <cmath>
#include <algorithm>

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
    // 1. Crear la ventana (Sintaxis SFML 3 con sf::VideoMode)
    sf::RenderWindow window(sf::VideoMode({1000, 800}), "Serie de Fourier - Letra F");
    window.setFramerateLimit(60);

    // 2. Puntos de la letra "F" (un solo trazo continuo para mejores resultados)
    std::vector<std::complex<double>> points = {
        {0, 0}, {100, 0}, {100, 20}, {20, 20}, {20, 50}, 
        {80, 50}, {80, 70}, {20, 70}, {20, 120}, {0, 120}, {0,0}
    };

    // Escalar y centrar un poco la letra
    for (auto& p : points) { p *= 2.5; }

    // 3. Obtener coeficientes y ordenar por tamaño de epiciclo
    std::vector<Coefficient> fourier = computeDFT(points);
    std::sort(fourier.begin(), fourier.end(), [](const Coefficient& a, const Coefficient& b) {
        return a.amplitude > b.amplitude;
    });

    std::vector<sf::Vector2f> path; 
    float time = 0;
    const float dt = (2.0f * static_cast<float>(M_PI)) / static_cast<float>(points.size());

    while (window.isOpen()) {
        // Manejo de eventos estilo SFML 3
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color(20, 20, 25));

        // 4. Calcular y dibujar Epiciclos
        sf::Vector2f currentPos(450, 300); // Punto de inicio en pantalla
        
        for (const auto& coeff : fourier) {
            sf::Vector2f prevPos = currentPos;
            
            float radius = static_cast<float>(coeff.amplitude);
            float angle = static_cast<float>(coeff.phase) + static_cast<float>(coeff.freq) * time;
            
            currentPos.x += radius * std::cos(angle);
            currentPos.y += radius * std::sin(angle);

            // Dibujar el círculo de la órbita
            sf::CircleShape circle(radius);
            circle.setFillColor(sf::Color::Transparent);
            circle.setOutlineColor(sf::Color(150, 150, 150, 50));
            circle.setOutlineThickness(1.0f);
            circle.setOrigin({radius, radius});
            circle.setPosition(prevPos);
            window.draw(circle);

            // Dibujar el radio (Conector) - SINTAXIS SFML 3 (Agregados)
            sf::Vertex line[] = { 
                {prevPos, sf::Color(255, 255, 255, 150)}, 
                {currentPos, sf::Color::White} 
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }

        // Guardar el rastro
        path.push_back(currentPos);

        // 5. Dibujar la "F" trazada
        if (path.size() > 1) {
            std::vector<sf::Vertex> vertexPath;
            for (const auto& p : path) {
                vertexPath.push_back({p, sf::Color::Cyan}); // SINTAXIS SFML 3
            }
            window.draw(vertexPath.data(), vertexPath.size(), sf::PrimitiveType::LineStrip);
        }

        window.display();

        // Avanzar el tiempo
        time += 0.02f; // Velocidad de la animación
        
        // Resetear cuando completa el ciclo para no saturar la memoria
        if (time > 2.0f * M_PI) {
            time = 0;
            path.clear();
        }
    }

    return 0;
}