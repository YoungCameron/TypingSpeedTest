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
    sf::Text globalTimerText;
    sf::Text accuracyText;
    sf::String pInput;
    sf::Clock promptClock;
    sf::Clock globalClock;
    std::string prompt = "The quick brown fox jumps over the lazy dog";
    std::string accuracyString;
    std::string accuracyStringFrac;
    std::fstream input;
    std::string promptLine;
    std::ostringstream timeStream;
    std::ostringstream accuracyStream;
    bool newChallenge = true;
    bool pausePromptTimer = false;
    bool beginGlobal = true;
    bool gameStart = true;
    bool gameOver = false;
    std::vector<float> timeStorage;
    std::vector<std::string> promptStorage;
    int promptFileLine = 1;
    static double correct;
    static double incorrect;
    float currentGlobalTime;

    // Detect input and handle accordingly
    void handleInput() {
        currentGlobalTime = globalClock.getElapsedTime().asSeconds();
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
        if (!pausePromptTimer) {
            timer();
        }
        cycleChallenge();
        timeStream << std::fixed << std::setprecision(2) << currentGlobalTime;
        globalTimerText.setString(timeStream.str());
        timeStream.str("");
    }

    // Render initial window and text
    void render() {
        window.clear(sf::Color(56, 68, 79, 255));
        window.draw(backgroundRectangle);
        window.draw(promptText);
        window.draw(globalTimerText);
        window.draw(accuracyText);
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
                correct++;
            } else {
                charText.setFillColor(sf::Color::Red);
                incorrect++;
            }

            window.draw(charText);
        }

        window.display();
    }

    // Keep track of start -> finish for accuracy and speed
    void timer() {
        // Clocks start by default, so we must pause and reset at start of game
        if (gameStart) {
            gameStart = false;
            globalClock.stop();
            globalClock.reset();
            promptClock.stop();
            promptClock.reset();
        }
        if (pInput.getSize() > 0 && beginGlobal) {
            beginGlobal = false;
            globalClock.reset();
            globalClock.start();
        }
        if (currentGlobalTime >= 10.00) {
            globalClock.stop();
            pInput.clear();
            gameEnd();
        }
        if (pInput.getSize() > 0 && newChallenge) {
            promptClock.reset();
            promptClock.start();
            newChallenge = false;
        } else if (pInput.getSize() == promptText.getString().getSize()) {
            promptClock.stop();
            pausePromptTimer = true;
            float totalTime = promptClock.getElapsedTime().asSeconds();
            timeStorage.push_back(totalTime);
            promptClock.reset();
            for (const float i: timeStorage) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }
    }

    void cycleChallenge() {
        if (!newChallenge && pInput.getSize() == promptText.getString().getSize()) {
            newChallenge = true;
            pausePromptTimer = false;
            pInput.clear();

            prompt = promptStorage[promptFileLine];
            promptFileLine++;
            promptText.setString(prompt);
        }
    }

    void gameEnd() {
        if (!gameOver) {
            gameOver = true;
            double accuracy = correct / (incorrect + correct);
            std::cout << std::fixed << std::setprecision(2) << "Accuracy: " << accuracy * 100;
            incorrect = 0;
            correct = 0;

            accuracyString = "Accuracy: ";
            accuracyStream << std::fixed << std::setprecision(2) << accuracy * 100;
            accuracyString.append((accuracyStream.str()));
            accuracyString.append("%");
            accuracyText.setString(accuracyString);
        }
    }

public:
    Game() : window(sf::VideoMode({1200, 700}), "Typing Speed Test", sf::Style::Close, sf::State::Windowed),
             promptText(font), globalTimerText(font), accuracyText(font) {
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

        globalTimerText.setPosition({800, 380});
        globalTimerText.setFillColor(sf::Color::White);
        globalTimerText.setCharacterSize(24);

        accuracyText.setPosition({500, 380});
        accuracyText.setFillColor(sf::Color::White);
        accuracyText.setCharacterSize(24);
        accuracyText.setStyle(sf::Text::Bold);
        accuracyText.setString("Accuracy: %");

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

double Game::correct = 0;
double Game::incorrect = 0;

int main() {
    Game game;
    game.run();
    return 0;
}
