#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <fstream>

class Game {
private:
    sf::RenderWindow window;
    sf::RectangleShape backgroundRectangle;
    sf::Font font;
    sf::Text promptText;
    sf::String pInput;
    sf::Clock promptClock;
    sf::Clock globalClock;
    std::string prompt = "The quick brown fox jumps over the lazy dog";
    bool newChallenge = true;
    bool pauseTimer = false;
    static std::vector<float> timeStorage;
    static std::vector<std::string> promptStorage;
    std::fstream input;
    std::string promptLine;
    static int promptFileLine;

    // Detect input and handle accordingly
    void handleInput() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto *textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (textEntered->unicode != '\b') {
                    pInput += textEntered->unicode;
                }
            } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace) {
                    if (!pInput.isEmpty()) {
                        pInput.erase(pInput.getSize() - 1);
                    }
                }
            }
        }
        if (!pauseTimer) {
            timer();
        }
        cycleChallenege();
    }

    // Render initial window and text
    void render() {
        window.clear(sf::Color(56, 68, 79, 255));
        window.draw(backgroundRectangle);
        window.draw(promptText);
        backgroundRectangle.setFillColor(sf::Color(101, 105, 110, 125));
        backgroundRectangle.setSize({700, 250});
        backgroundRectangle.setPosition({240, 200});

        for (int i = 0; i < pInput.getSize(); i++) {
            sf::Text charText(font);
            charText.setString(pInput[i]);
            charText.setCharacterSize(24);
            charText.setStyle(sf::Text::Bold);
            charText.setPosition(promptText.findCharacterPos(i));

            if (pInput[i] == promptText.getString()[i]) {
                charText.setFillColor(sf::Color::Green);
            } else {
                charText.setFillColor(sf::Color::Red);
            }

            window.draw(charText);
        }

        window.display();
    }

    // Keep track of start -> finish for accuracy and speed
    void timer() {
        if (pInput.getSize() > 0 && newChallenge) {
            promptClock.start();
            newChallenge = false;
        } else if (pInput.getSize() == promptText.getString().getSize()) {
            promptClock.stop();
            pauseTimer = true;
            float totalTime = promptClock.getElapsedTime().asSeconds();
            timeStorage.push_back(totalTime);
            promptClock.reset();
            for (const float i : timeStorage) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }
    }

    void cycleChallenege() {
        if (!newChallenge && pInput.getSize() == promptText.getString().getSize()) {
            newChallenge = true;
            pauseTimer = false;
            pInput.clear();

            prompt = promptStorage[promptFileLine];
            promptFileLine++;
            promptText.setString(prompt);
        }

    }

public:
    Game() : window(sf::VideoMode({1200, 700}), "Typing Speed Test", sf::Style::Close, sf::State::Windowed),
             promptText(font) {
        if (!font.openFromFile("SNPro.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
            return;
        }

        sf::Image icon;
        icon.loadFromFile("STT.jpeg");
        window.setIcon(icon.getSize(), icon.getPixelsPtr());
        window.setFramerateLimit(60);

        promptText.setFont(font);
        promptText.setString(prompt);
        promptText.setCharacterSize(24);
        promptText.setFillColor(sf::Color::White);
        promptText.setStyle(sf::Text::Bold);
        promptText.setPosition({360, 250});

        input.open("textPrompts.txt");
        while (std::getline(input, promptLine)) {
            promptStorage.push_back(promptLine);
        }
        input.close();
    }

    void run() {
        while (window.isOpen()) {
            handleInput();
            render();
        }
    }
};

std::vector<float> Game::timeStorage = {};
std::vector<std::string> Game::promptStorage = {};
int Game::promptFileLine = 1;

int main() {
    Game game;
    game.run();
    return 0;
}
