#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <fstream>
#include <format>
#include "GameMenu.h"

enum GameState { WAITING, PLAYING, RESULTS };

class Game {
private:
    GameState state = WAITING;
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
    std::string wpmString;
    std::string promptLine;
    std::fstream input;
    bool newChallenge = true;
    bool pausePromptTimer = false;
    bool beginGlobal = true;
    std::vector<float> timeStorage;
    std::vector<std::string> promptStorage;
    int promptFileLine = 1;
    int textPosition = 0; // For accuracy
    double characterCount = 0; // For WPM
    static double correct;
    static double incorrect;
    double wpmCalculation;
    double accuracy;
    float currentGlobalTime;

    // Detect input and handle accordingly
    void handleInput() {
        currentGlobalTime = globalClock.getElapsedTime().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (state == WAITING || state == PLAYING) {
                if (const auto *textEntered = event->getIf<sf::Event::TextEntered>()) {
                    characterCount++;
                    if (textEntered->unicode != '\b' && textEntered->unicode != '\r') {
                        pInput += textEntered->unicode;
                        if (textEntered->unicode == promptText.getString()[textPosition]) {
                            correct++;
                        } else {
                            incorrect++;
                        }
                        textPosition++;
                    }
                } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace) {
                        if (!pInput.isEmpty()) {
                            textPosition--;
                            pInput.erase(pInput.getSize() - 1);
                        }
                    }
                }
            }
            // Reset the game if the user hits enter during RESULTS state
            if (state == RESULTS) {
                if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                        resetGame();
                    }
                }
            }
        }
        // Start timer
        if (!pausePromptTimer) {
            timer();
        }
        cycleChallenge();
        globalTimerText.setString(std::format("{:.2f}",currentGlobalTime));

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
            } else {
                charText.setFillColor(sf::Color::Red);
            }

            window.draw(charText);
        }

        window.display();
    }

    // Clocks start by default, so we must pause and reset at start of game
    void resetClocks() {
        if (state == WAITING) {
            state = PLAYING;
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
        if (currentGlobalTime >= 30.00) {
            globalClock.stop();
            gameEnd();
            pInput.clear();
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
            textPosition = 0;
            pInput.clear();

            prompt = promptStorage[promptFileLine];
            promptFileLine++;
            if (prompt.size() > 44) {
                int location = 44;
                while (true) {
                    constexpr char space = ' ';
                    if (prompt[location] != space) {
                        location++;
                    } else {
                        prompt.replace(location, 1, "\n");
                        break;
                    }
                }
            }
            promptText.setString(prompt);
        }
    }

    // At the end of the game, change game state and calculate/display Accuracy and WPM on window
    void gameEnd() {

        if (state == PLAYING) {
            state = RESULTS;

            accuracy = correct / (incorrect + correct);
            wpmCalculation = (characterCount / 5) / (globalClock.getElapsedTime().asSeconds() / 60);

            wpmString = std::format("WPM: {:.0f}", wpmCalculation);
            wpmText.setString(wpmString);

            accuracyString = std::format("Accuracy: {:.0f}%", accuracy*100);
            accuracyText.setString(accuracyString);
        }
    }


    // Reset flags, text, and variables for a new game
    void resetGame() {
        accuracyString = "Accuracy: %";
        wpmString = "WPM: ";
        accuracyText.setString(accuracyString);
        wpmText.setString(wpmString);
        characterCount = 0;
        correct = 0;
        incorrect = 0;
        currentGlobalTime = 0;
        textPosition = 0;
        newChallenge = true;
        pausePromptTimer = false;
        beginGlobal = true;
        state = WAITING;
    }

public:
    Game() : window(sf::VideoMode({1200, 700}), "Typing Speed Test", sf::Style::Close, sf::State::Windowed),
             promptText(font), globalTimerText(font), accuracyText(font), wpmText(font), wpmCalculation(), currentGlobalTime() {
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
