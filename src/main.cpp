#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <fstream>
#include "GameMenu.h"

class Game {
private:
    sf::RenderWindow window;
    sf::RectangleShape backgroundRectangle;
    sf::Font font;
    sf::Text promptText;
    sf::Text globalTimerText;
    sf::Text accuracyText;
    sf::Text wpmText;
    sf::String pInput;
    sf::Clock promptClock;
    sf::Clock globalClock;
    std::string prompt = "The quick brown fox jumps over the lazy dog";
    std::string accuracyString;
    std::string accuracyStringFrac;
    std::string wpmString;
    std::fstream input;
    std::string promptLine;
    std::ostringstream timeStream;
    std::ostringstream accuracyStream;
    std::ostringstream wpmStream;
    bool newChallenge = true;
    bool pausePromptTimer = false;
    bool beginGlobal = true;
    bool gameStart = true;
    bool gameOver = false;
    std::vector<float> timeStorage;
    std::vector<std::string> promptStorage;
    int promptFileLine = 1;
    double characterCount = 0;
    static double correct;
    static double incorrect;
    double wpmCalculation;
    float currentGlobalTime;

    // Detect input and handle accordingly
    void handleInput() {
        currentGlobalTime = globalClock.getElapsedTime().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                characterCount++;
                if (textEntered->unicode != '\b') {
                    pInput += textEntered->unicode;
                }
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace) {
                    if (!pInput.isEmpty()) {
                        pInput.erase(pInput.getSize() - 1);
                    }
                }
            }
            if (gameOver) { // TODO: Implement this if statement correctly
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed -> scancode == sf::Keyboard::Scancode::Enter) {

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
        window.draw(wpmText);

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

    void resetClocks() {
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
    }

    // Keep track of start -> finish for accuracy and speed
    void timer() {
        resetClocks();
        if (currentGlobalTime >= 5.00) {
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
        }
    }

    void cycleChallenge() {
        if (!newChallenge && pInput.getSize() == promptText.getString().getSize()) {
            newChallenge = true;
            pausePromptTimer = false;
            pInput.clear();

            prompt = promptStorage[promptFileLine];
            promptFileLine++;
            if (prompt.size() > 44) {
                int location = 44;
                while (true) {
                    const char space = ' ';
                    if (prompt[location] != space) {
                        location++;
                    } else {
                        prompt.replace(location,1 ,"\n");
                        break;
                    }
                }
            }
            promptText.setString(prompt);
        }
    }

    void gameEnd() {
        if (!gameOver) {
            gameOver = true;
            double accuracy = correct / (incorrect + correct);
            incorrect = 0;
            correct = 0;
            wpmCalculation = (characterCount / 5) / (globalClock.getElapsedTime().asSeconds() / 60);

            accuracyString = "Accuracy: ";
            accuracyStream << std::fixed << std::setprecision(2) << accuracy * 100;
            accuracyString.append((accuracyStream.str()));
            accuracyString.append("%");
            accuracyText.setString(accuracyString);

            wpmStream << std::fixed << std::setprecision(0) << wpmCalculation;
            wpmString.append("WPM: ");
            wpmString.append(wpmStream.str());
            wpmText.setString(wpmString);

        }
    }

public:
    Game() : window(sf::VideoMode({1200, 700}), "Typing Speed Test", sf::Style::Close, sf::State::Windowed),
             promptText(font), globalTimerText(font), accuracyText(font), wpmText(font) {
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
        promptText.setPosition({310, 250});

        globalTimerText.setPosition({800, 380});
        globalTimerText.setFillColor(sf::Color::White);
        globalTimerText.setCharacterSize(24);

        accuracyText.setPosition({500, 380});
        accuracyText.setFillColor(sf::Color::White);
        accuracyText.setCharacterSize(24);
        accuracyText.setStyle(sf::Text::Bold);
        accuracyText.setString("Accuracy: %");

        wpmText.setPosition({320, 380});
        wpmText.setFillColor(sf::Color::White);
        wpmText.setCharacterSize(24);
        wpmText.setStyle(sf::Text::Bold);
        wpmText.setString("WPM: ");

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
