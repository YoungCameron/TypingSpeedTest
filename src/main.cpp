#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <fstream>
#include <format>

enum GameState { WAITING, PLAYING, RESULTS, MAINMENU, OPTIONS, STATS, PHRASEMENU };
enum SelectionMenu { PLAY, SETTINGS, STAT };
enum OptionsMenu { setPlayTime, setFPS, setPhrases };

class Game {
private:
    GameState state = MAINMENU;
    SelectionMenu selectionMenu = PLAY;
    OptionsMenu optionsMenu = setPlayTime;
    sf::RenderWindow window;
    sf::RectangleShape backgroundRectangle;
    sf::Font font;
    sf::Text promptText;
    sf::Text globalTimerText;
    sf::Text accuracyText;
    sf::Text wpmText;
    // menuOption handles all menu options
    sf::Text menuOption1;
    sf::Text menuOption2;
    sf::Text menuOption3;
    sf::Text phraseMenuNumber;
    sf::String promptInput;
    sf::Clock promptClock;
    sf::Clock globalClock;
    std::string prompt = "The quick brown fox jumps over the lazy dog";
    std::string accuracyString;
    std::string wpmString;
    std::string promptLine;
    int phrasePageNumber = 1;
    std::fstream input;
    bool newChallenge = true;
    bool pausePromptTimer = false;
    bool beginGlobalClock = true;
    std::vector<float> timeStorage;
    std::vector<std::string> promptStorage;
    int promptFileLine = 1;
    int textPosition = 0; // For accuracy
    int correct = 0;
    int incorrect = 0;
    int characterCount = 0; // For WPM
    int FPS = 60;
    double wpmCalculation = 0;
    double accuracy = 0;
    float currentGlobalTime;
    float playTime = 15.0;

    // Detect input and handle accordingly
    void handleInput() {
        currentGlobalTime = globalClock.getElapsedTime().asSeconds();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (state == MAINMENU) {
                if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scan::Enter) {
                        if (selectionMenu == PLAY) {
                            state = WAITING;
                        }
                        if (selectionMenu == STAT) {
                            // TODO
                        }
                        if (selectionMenu == SETTINGS) {
                            state = OPTIONS;
                        }
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scan::Down) {
                        if (selectionMenu == PLAY) {
                            selectionMenu = STAT;
                        } else if (selectionMenu == STAT) {
                            selectionMenu = SETTINGS;
                        } else {
                            selectionMenu = PLAY;
                        }
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scan::Up) {
                        if (selectionMenu == PLAY) {
                            selectionMenu = SETTINGS;
                        } else if (selectionMenu == SETTINGS) {
                            selectionMenu = STAT;
                        } else {
                            selectionMenu = PLAY;
                        }
                    }
                }
            } else if (state == OPTIONS) {
                if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scan::Enter) {
                        if (optionsMenu == setPlayTime) {
                            switch (static_cast<int>(playTime)) {
                                case 15:
                                    playTime = 30;
                                    break;
                                case 30:
                                    playTime = 60;
                                    break;
                                case 60:
                                    playTime = 90;
                                    break;
                                default:
                                    playTime = 15;
                            }
                            menuOption1.setString("Play Time " + std::to_string(static_cast<int>(playTime)));
                        }
                        if (optionsMenu == setFPS) {
                            switch (FPS) {
                                case 60:
                                    FPS = 120;
                                    break;
                                default:
                                    FPS = 60;
                            }
                            window.setFramerateLimit(FPS);
                            menuOption2.setString("FPS " + std::to_string(FPS));
                        }
                        if (optionsMenu == setPhrases) {
                            state = PHRASEMENU; // TODO: Implement a phrases edit menu
                        }
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scan::Escape) {
                        state = MAINMENU;
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scan::Down) {
                        if (optionsMenu == setPlayTime) {
                            optionsMenu = setFPS;
                        } else if (optionsMenu == setFPS) {
                            optionsMenu = setPhrases;
                        } else {
                            optionsMenu = setPlayTime;
                        }
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scan::Up) {
                        if (optionsMenu == setPlayTime) {
                            optionsMenu = setPhrases;
                        } else if (optionsMenu == setPhrases) {
                            optionsMenu = setFPS;
                        } else {
                            optionsMenu = setPlayTime;
                        }
                    }
                }
            } else if (state == WAITING || state == PLAYING) {
                if (const auto *textEntered = event->getIf<sf::Event::TextEntered>()) {
                    characterCount++;
                    if (textEntered->unicode != '\b' && textEntered->unicode != '\r') {
                        promptInput += textEntered->unicode;
                        if (textPosition < promptText.getString().getSize()) {
                            if (textEntered->unicode == promptText.getString()[textPosition]) {
                                correct++;
                            } else {
                                incorrect++;
                            }
                        }
                        textPosition++;
                    }
                }
                if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Backspace) {
                        if (!promptInput.isEmpty()) {
                            textPosition--;
                            promptInput.erase(promptInput.getSize() - 1);
                        }
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                        resetGame();
                        state = MAINMENU;
                    }
                }

            }
            if (state == PHRASEMENU) {
                if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
                        if (phrasePageNumber != promptStorage.size()) {
                            phrasePageNumber++;
                        } else {
                            phrasePageNumber = 1;
                        }
                    }
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Left) {
                        if (phrasePageNumber != 1) {
                            phrasePageNumber--;
                        } else {
                            phrasePageNumber = static_cast<int>(promptStorage.size());
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
        globalTimerText.setString(std::format("{:.2f}", currentGlobalTime));
    }

    // Render initial window and text
    void render() {
        window.clear(sf::Color(56, 68, 79, 255));
        if (state == MAINMENU) {
            menuOption1.setCharacterSize(17);
            menuOption1.setPosition({500, 275});
            menuOption1.setStyle(sf::Text::Bold);
            menuOption1.setFillColor(sf::Color::White);
            menuOption1.setString("Play");

            menuOption3.setCharacterSize(17);
            menuOption3.setPosition({500, 300});
            menuOption3.setStyle(sf::Text::Bold);
            menuOption3.setFillColor(sf::Color::White);
            menuOption3.setString("Stats");

            menuOption2.setCharacterSize(17);
            menuOption2.setPosition({500, 325});
            menuOption2.setStyle(sf::Text::Bold);
            menuOption2.setFillColor(sf::Color::White);
            menuOption2.setString("Options");

            if (selectionMenu == PLAY) {
                menuOption1.setFillColor(sf::Color::Green);
            }
            if (selectionMenu == STAT) {
                menuOption3.setFillColor(sf::Color::Green);
            }
            if (selectionMenu == SETTINGS) {
                menuOption2.setFillColor(sf::Color::Green);
            }

            window.draw(menuOption1);
            window.draw(menuOption2);
            window.draw(menuOption3);
        }
        if (state == OPTIONS) {
            menuOption1.setCharacterSize(17);
            menuOption1.setPosition({500, 275});
            menuOption1.setStyle(sf::Text::Bold);
            menuOption1.setFillColor(sf::Color::White);
            menuOption1.setString("Play Time " + std::to_string(static_cast<int>(playTime)));


            menuOption2.setCharacterSize(17);
            menuOption2.setPosition({500, 300});
            menuOption2.setStyle(sf::Text::Bold);
            menuOption2.setFillColor(sf::Color::White);
            menuOption2.setString("FPS " + std::to_string(FPS));

            menuOption3.setCharacterSize(17);
            menuOption3.setPosition({500, 325});
            menuOption3.setStyle(sf::Text::Bold);
            menuOption3.setFillColor(sf::Color::White);
            menuOption3.setString("Phrases");

            if (optionsMenu == setPlayTime) {
                menuOption1.setFillColor(sf::Color::Green);
            }
            if (optionsMenu == setFPS) {
                menuOption2.setFillColor(sf::Color::Green);
            }
            if (optionsMenu == setPhrases) {
                menuOption3.setFillColor(sf::Color::Green);
            }

            window.draw(menuOption1);
            window.draw(menuOption2);
            window.draw(menuOption3);
        }
        if (state == WAITING || state == PLAYING || state == RESULTS) {
            window.draw(promptText);
            window.draw(globalTimerText);
            window.draw(accuracyText);
            window.draw(wpmText);

            backgroundRectangle.setFillColor(sf::Color(101, 105, 110, 125));
            backgroundRectangle.setSize({700, 250});
            backgroundRectangle.setPosition({240, 200});
            window.draw(backgroundRectangle);

            for (int i = 0; i < promptInput.getSize(); i++) {
                sf::Text charText(font);
                charText.setString(promptInput[i]);
                charText.setCharacterSize(24);
                charText.setStyle(sf::Text::Bold);
                charText.setPosition(promptText.findCharacterPos(i));

                if (promptInput[i] == promptText.getString()[i]) {
                    charText.setFillColor(sf::Color::Green);
                } else {
                    charText.setFillColor(sf::Color::Red);
                }

                window.draw(charText);
            }
        } if (state == PHRASEMENU) {
            backgroundRectangle.setFillColor(sf::Color(101, 105, 110, 125));
            backgroundRectangle.setSize({700, 250});
            backgroundRectangle.setPosition({240, 200});
            window.draw(backgroundRectangle);

            phraseMenuNumber.setPosition({580, 225});
            phraseMenuNumber.setFillColor(sf::Color::White);
            phraseMenuNumber.setCharacterSize(14);
            phraseMenuNumber.setString(std::to_string(phrasePageNumber) + "/" + std::to_string(static_cast<int>(promptStorage.size())));
            window.draw(phraseMenuNumber);
        }

        window.display();
    }

    // Clocks start by default, so we must pause and reset at start of game
    void resetClocks() {
        if (state == WAITING) {
            globalClock.stop();
            globalClock.reset();
            promptClock.stop();
            promptClock.reset();
        }
        if (promptInput.getSize() > 0 && beginGlobalClock) {
            beginGlobalClock = false;
            globalClock.reset();
            globalClock.start();
        }
    }

    // Keep track of start -> finish for accuracy and speed
    void timer() {
        resetClocks();
        if (currentGlobalTime >= playTime) {
            globalClock.stop();
            gameEnd();
            promptInput.clear();
        }
        if (promptInput.getSize() > 0 && newChallenge) {
            state = PLAYING;
            promptClock.reset();
            promptClock.start();
            newChallenge = false;
        } else if (promptInput.getSize() == promptText.getString().getSize()) {
            promptClock.stop();
            pausePromptTimer = true;
            float totalTime = promptClock.getElapsedTime().asSeconds();
            timeStorage.push_back(totalTime);
            promptClock.reset();
        }
    }

    void cycleChallenge() {
        if (!newChallenge && promptInput.getSize() == promptText.getString().getSize()) {
            newChallenge = true;
            pausePromptTimer = false;
            textPosition = 0;
            promptInput.clear();

            prompt = promptStorage[promptFileLine];
            promptFileLine++;
            if (promptFileLine >= promptStorage.size()) {
                promptFileLine = 0;
            }
            if (prompt.size() > 44) {
                int location = 44;
                while (true) {
                    constexpr char space = ' ';
                    if (prompt[location] != space && location < prompt.size() - 1) {
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

            accuracy = static_cast<double>(correct) / (incorrect + correct);
            wpmCalculation = (static_cast<float>(characterCount) / 5) / (globalClock.getElapsedTime().asSeconds() / 60);

            wpmString = std::format("WPM: {:.0f}", wpmCalculation);
            wpmText.setString(wpmString);

            accuracyString = std::format("Accuracy: {:.0f}%", accuracy * 100);
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
        beginGlobalClock = true;
        state = WAITING;
    }

public:
    Game() : window(sf::VideoMode({1200, 700}), "Typing Speed Test", sf::Style::Close, sf::State::Windowed),
             promptText(font), globalTimerText(font), accuracyText(font), wpmText(font), menuOption1(font), menuOption2(font),
             menuOption3(font), phraseMenuNumber(font), currentGlobalTime() {
        if (!font.openFromFile("SNPro.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
            return;
        }

        sf::Image icon;
        icon.loadFromFile("STT.jpeg");
        window.setIcon(icon.getSize(), icon.getPixelsPtr());
        window.setFramerateLimit(FPS);

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

int main() {
    Game game;
    game.run();
    return 0;
}
