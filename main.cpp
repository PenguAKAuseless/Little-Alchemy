#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <iostream>

/*
Compilation instructions:
g++ -c main.cpp -o main.o -std=c++17
g++ main.o -o game -lsfml-graphics -lsfml-window -lsfml-system
*/

/**
 * Element class represents discoverable elements in the game
 * Each element has a name, description, discovery status, and creation count
 */
class Element
{
public:
    std::string name;        // Element name (e.g., "Fire", "Air")
    std::string description; // Descriptive text for the element
    bool discovered;         // Whether the player has discovered this element
    int creationCount;       // How many times this element has been created

    Element(const std::string &n, const std::string &desc, bool disc = false)
        : name(n), description(desc), discovered(disc), creationCount(0) {}
};

/**
 * GameObject class represents interactive element instances in the game world
 * These are the draggable sprites that players can combine
 */
class GameObject
{
public:
    std::shared_ptr<Element> element; // Reference to the element type
    sf::Sprite sprite;                // Visual representation
    std::string spritePath;           // Path to the sprite image file
    float creationTime;               // When this object was created (for cleanup)
    bool isDragging;                  // Whether this object is currently being dragged

    GameObject(std::shared_ptr<Element> elem, const sf::Texture &texture, const std::string &path, sf::Vector2f pos, float time)
        : element(elem), spritePath(path), creationTime(time), isDragging(false)
    {
        sprite.setTexture(texture);
        sprite.setPosition(pos);
        sprite.setScale(0.5f, 0.5f); // Scale down sprites to 50% size
    }
};

/**
 * CombinationRegistry manages valid element combinations and their results
 * Stores recipes for creating new elements from existing ones
 */
class CombinationRegistry
{
    // Map storing element pairs and their combination results
    std::map<std::pair<std::string, std::string>, std::string> combinations;

public:
    CombinationRegistry()
    {
        // Basic Element Combinations (6 combinations)
        combinations[{"Fire", "Water"}] = "Steam";
        combinations[{"Water", "Fire"}] = "Steam";

        combinations[{"Fire", "Earth"}] = "Lava";
        combinations[{"Earth", "Fire"}] = "Lava";

        combinations[{"Fire", "Air"}] = "Smoke";
        combinations[{"Air", "Fire"}] = "Smoke";

        combinations[{"Water", "Earth"}] = "Mud";
        combinations[{"Earth", "Water"}] = "Mud";

        combinations[{"Water", "Air"}] = "Mist";
        combinations[{"Air", "Water"}] = "Mist";

        combinations[{"Earth", "Air"}] = "Dust";
        combinations[{"Air", "Earth"}] = "Dust";

        // Duplicate Element Combinations (4 combinations)
        combinations[{"Fire", "Fire"}] = "Energy";
        combinations[{"Water", "Water"}] = "Ocean";
        combinations[{"Earth", "Earth"}] = "Mountain";
        combinations[{"Air", "Air"}] = "Wind";

        // Advanced Combinations (12 combinations)
        combinations[{"Steam", "Air"}] = "Cloud";
        combinations[{"Air", "Steam"}] = "Cloud";

        combinations[{"Cloud", "Water"}] = "Rain";
        combinations[{"Water", "Cloud"}] = "Rain";

        combinations[{"Mud", "Energy"}] = "Plant";
        combinations[{"Energy", "Mud"}] = "Plant";

        combinations[{"Lava", "Air"}] = "Stone";
        combinations[{"Air", "Lava"}] = "Stone";

        combinations[{"Lava", "Mountain"}] = "Volcano";
        combinations[{"Mountain", "Lava"}] = "Volcano";

        combinations[{"Energy", "Air"}] = "Lightning";
        combinations[{"Air", "Energy"}] = "Lightning";

        combinations[{"Water", "Wind"}] = "Ice";
        combinations[{"Wind", "Water"}] = "Ice";

        combinations[{"Stone", "Wind"}] = "Sand";
        combinations[{"Wind", "Stone"}] = "Sand";

        combinations[{"Mud", "Plant"}] = "Swamp";
        combinations[{"Plant", "Mud"}] = "Swamp";

        combinations[{"Plant", "Plant"}] = "Forest";

        combinations[{"Sand", "Sand"}] = "Desert";

        combinations[{"Energy", "Plant"}] = "Life";
        combinations[{"Plant", "Energy"}] = "Life";
    }

    /**
     * Check if two elements can be combined together
     */
    bool isValidCombination(const std::string &e1, const std::string &e2) const
    {
        return combinations.find({e1, e2}) != combinations.end();
    }

    /**
     * Get the result of combining two elements
     * Returns empty string if combination is invalid
     */
    std::string getResult(const std::string &e1, const std::string &e2) const
    {
        auto it = combinations.find({e1, e2});
        if (it != combinations.end())
            return it->second;
        return "";
    }
};

/**
 * ElementBook class manages the encyclopedia/book interface
 * Shows discovered elements with their details and descriptions
 */
class ElementBook
{
    std::vector<std::shared_ptr<Element>> elements;     // List of all elements
    sf::Font font;                                      // Font for text rendering
    const std::map<std::string, sf::Texture> &textures; // Reference to game textures
    bool isOpen;                                        // Whether the book is currently open
    int selectedIndex;                                  // Currently selected element index
    sf::Texture crossTex;                               // Close button texture
    sf::Texture bookTex;                                // Book icon texture
    sf::Sprite bookIcon;                                // Clickable book icon sprite
    sf::Text welcomeText;                               // Welcome message when no element selected
    float bookScroll = 0.0f;                            // Scroll offset for book sidebar
    const float bookScrollSpeed = 30.0f;                // Pixels per scroll step

public:
    ElementBook(const std::map<std::string, sf::Texture> &tex) : textures(tex), isOpen(false), selectedIndex(-1)
    {
        // Load font for text rendering
        if (!font.loadFromFile("fonts/Pixel Game.otf"))
        {
            std::cerr << "Failed to load font from fonts/Pixel Game.otf, using fallback fonts/arial.ttf\n";
            font.loadFromFile("fonts/arial.ttf");
        }

        // Load close button texture (X icon)
        if (!crossTex.loadFromFile("assets/cross.png"))
        {
            std::cerr << "Failed to load close icon: assets/cross.png\n";
            // Create fallback black square if texture fails to load
            sf::Image img;
            img.create(32, 32, sf::Color::Black);
            crossTex.loadFromImage(img);
        }

        // Load book icon texture
        if (!bookTex.loadFromFile("assets/book.png"))
        {
            std::cerr << "Failed to load book icon: assets/book.png\n";
            // Create fallback green square if texture fails to load
            sf::Image img;
            img.create(30, 30, sf::Color::Green);
            bookTex.loadFromImage(img);
        }

        // Setup book icon sprite
        bookIcon.setTexture(bookTex);
        bookIcon.setPosition(10, 10);                                                // Top-left corner position
        bookIcon.setScale(64.0f / bookTex.getSize().x, 64.0f / bookTex.getSize().y); // Scale to 64x64 pixels

        // Initialize welcome text displayed when no element is selected
        welcomeText.setFont(font);
        welcomeText.setCharacterSize(22);
        welcomeText.setFillColor(sf::Color::Black);
        welcomeText.setString("Click on the icons to view elements descriptions");
    }

    /**
     * Add an element to the book's registry
     */
    void addElement(std::shared_ptr<Element> elem)
    {
        elements.push_back(elem);
    }

    /**
     * Toggle the book open/closed state
     */
    void toggle()
    {
        isOpen = !isOpen;
        selectedIndex = -1; // Clear selection when toggling
    }

    bool isBookOpen() const { return isOpen; }

    /**
     * Provide access to the cross texture for other classes
     */
    const sf::Texture &getCrossTexture() const { return crossTex; }

    /**
     * Handle mouse input for book interactions
     */
    void handleInput(const sf::Event &event, const sf::RenderWindow &window)
    {
        // Handle book icon click when book is closed
        if (!isOpen)
        {
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                if (bookIcon.getGlobalBounds().contains(mousePos))
                {
                    toggle();
                }
            }
            return;
        }

        // Handle interactions when book is open
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

            // Close book if clicking outside the book area
            if (mousePos.x < 100 || mousePos.x > 700 || mousePos.y < 100 || mousePos.y > 500)
            {
                toggle();
                return;
            }

            // Handle close button (X) click
            sf::RectangleShape exitButton(sf::Vector2f(32, 32));
            exitButton.setPosition(668, 100); // Top-right corner of book
            if (exitButton.getGlobalBounds().contains(mousePos))
            {
                toggle();
                return;
            }

            // Handle element selection in the sidebar
            for (size_t i = 0; i < elements.size(); ++i)
            {
                float yPos = 100 + i * 30 - bookScroll;
                sf::RectangleShape clickArea(sf::Vector2f(100, 30));
                clickArea.setPosition(130, yPos);

                // Only check if visible
                if (yPos >= 70 && yPos <= 500 && clickArea.getGlobalBounds().contains(mousePos))
                {
                    selectedIndex = static_cast<int>(i);
                    return;
                }
            }
        }

        // Handle scroll wheel for book sidebar
        if (event.type == sf::Event::MouseWheelScrolled && isOpen)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y));

            // Check if mouse is over book sidebar
            if (mousePos.x >= 100 && mousePos.x <= 200 && mousePos.y >= 100 && mousePos.y <= 500)
            {
                bookScroll -= event.mouseWheelScroll.delta * bookScrollSpeed;

                // Clamp scroll bounds
                float maxScroll = std::max(0.0f, elements.size() * 30.0f - 400 + 50);
                bookScroll = std::max(0.0f, std::min(bookScroll, maxScroll));
            }
        }
    }

    /**
     * Render the book interface
     */
    void draw(sf::RenderWindow &window, float time)
    {
        // Always draw the book icon
        window.draw(bookIcon);

        // Only draw book contents if open
        if (!isOpen)
            return;

        // Draw left sidebar (element list)
        sf::RectangleShape sidebar(sf::Vector2f(100, 400));
        sidebar.setPosition(100, 100);
        sidebar.setFillColor(sf::Color(251, 251, 251));
        window.draw(sidebar);

        // Draw main book area (element details)
        sf::RectangleShape bg(sf::Vector2f(500, 400));
        bg.setPosition(200, 100);
        bg.setFillColor(sf::Color(217, 234, 242));
        window.draw(bg);

        // Draw close button (X)
        sf::Sprite closeIcon(crossTex);
        closeIcon.setPosition(668, 100);
        closeIcon.setScale(32.0f / crossTex.getSize().x, 32.0f / crossTex.getSize().y);
        window.draw(closeIcon);

        // Draw element list in sidebar with scrolling
        for (size_t i = 0; i < elements.size(); ++i)
        {
            float yPos = 110 + i * 30 - bookScroll;

            // Only draw if visible
            if (yPos >= 100 && yPos <= 480)
            {
                sf::Sprite icon;
                if (elements[i]->discovered)
                {
                    auto it = textures.find(elements[i]->name);
                    if (it != textures.end())
                    {
                        icon.setTexture(it->second);
                        icon.setScale(20.0f / it->second.getSize().x, 20.0f / it->second.getSize().y);
                    }
                }
                else
                {
                    sf::Texture placeholder;
                    sf::Image img;
                    img.create(20, 20, sf::Color::White);
                    placeholder.loadFromImage(img);
                    icon.setTexture(placeholder);
                    icon.setScale(1.0f, 1.0f);
                }
                icon.setPosition(105, yPos);
                window.draw(icon);

                sf::Text text(elements[i]->discovered ? elements[i]->name : "???", font, 20);
                text.setPosition(130, yPos);
                text.setFillColor(sf::Color::Black);
                window.draw(text);
            }
        }

        // Draw selected element details in main area
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(elements.size()))
        {
            auto elem = elements[selectedIndex];

            // Draw large element icon
            sf::Sprite largeIcon;
            if (elem->discovered)
            {
                auto it = textures.find(elem->name);
                if (it != textures.end())
                {
                    largeIcon.setTexture(it->second);
                    largeIcon.setScale(200.0f / it->second.getSize().x, 200.0f / it->second.getSize().y);
                }
            }
            else
            {
                // Show large placeholder for undiscovered elements
                sf::Texture placeholder;
                sf::Image img;
                img.create(200, 200, sf::Color::White);
                placeholder.loadFromImage(img);
                largeIcon.setTexture(placeholder);
                largeIcon.setScale(1.0f, 1.0f);
            }
            largeIcon.setPosition(350, 125); // Centered in book area
            window.draw(largeIcon);

            // Draw element details text
            sf::Text details;
            details.setFont(font);
            details.setCharacterSize(18);
            details.setFillColor(sf::Color::Black);
            if (elem->discovered)
            {
                std::string formula;
                if (elem->name == "Steam")
                {
                    formula = "Fire + Water";
                }
                else if (elem->name == "Lava")
                {
                    formula = "Fire + Earth";
                }
                else if (elem->name == "Smoke")
                {
                    formula = "Fire + Air";
                }
                else if (elem->name == "Mud")
                {
                    formula = "Water + Earth";
                }
                else if (elem->name == "Mist")
                {
                    formula = "Water + Air";
                }
                else if (elem->name == "Dust")
                {
                    formula = "Earth + Air";
                }
                else if (elem->name == "Energy")
                {
                    formula = "Fire + Fire";
                }
                else if (elem->name == "Ocean")
                {
                    formula = "Water + Water";
                }
                else if (elem->name == "Mountain")
                {
                    formula = "Earth + Earth";
                }
                else if (elem->name == "Wind")
                {
                    formula = "Air + Air";
                }
                else if (elem->name == "Cloud")
                {
                    formula = "Steam + Air";
                }
                else if (elem->name == "Rain")
                {
                    formula = "Cloud + Water";
                }
                else if (elem->name == "Plant")
                {
                    formula = "Mud + Energy";
                }
                else if (elem->name == "Stone")
                {
                    formula = "Lava + Air";
                }
                else if (elem->name == "Volcano")
                {
                    formula = "Lava + Mountain";
                }
                else if (elem->name == "Lightning")
                {
                    formula = "Energy + Air";
                }
                else if (elem->name == "Ice")
                {
                    formula = "Water + Wind";
                }
                else if (elem->name == "Sand")
                {
                    formula = "Stone + Wind";
                }
                else if (elem->name == "Swamp")
                {
                    formula = "Mud + Plant";
                }
                else if (elem->name == "Forest")
                {
                    formula = "Plant + Plant";
                }
                else if (elem->name == "Desert")
                {
                    formula = "Sand + Sand";
                }
                else if (elem->name == "Life")
                {
                    formula = "Energy + Plant";
                }
                else
                {
                    formula = "Basic Element";
                }
                // Show full details for discovered elements
                details.setString("Name: " + elem->name + "\nCreated: " + std::to_string(elem->creationCount) +
                                  "\nDescription: " + elem->description + "\nFormula: " + formula);
            }
            else
            {
                // Show hidden details for undiscovered elements
                details.setString("Name: ???\nCreated: ???\nDescription: ???\nFormula: ???");
            }
            details.setPosition(325, 370);

            // Draw border around details area
            sf::RectangleShape border(sf::Vector2f(300, 125));
            border.setPosition(300, 350);
            border.setFillColor(sf::Color::Transparent);
            border.setOutlineColor(sf::Color::Black);
            border.setOutlineThickness(2);
            window.draw(border);
            window.draw(details);
        }
        else
        {
            // Show welcome text when no element is selected
            welcomeText.setPosition(450, 300); // Centered in book area
            sf::FloatRect textRect = welcomeText.getLocalBounds();
            welcomeText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            window.draw(welcomeText);
        }
    }
};

/**
 * Main Game class - orchestrates all game systems and handles the main game loop
 */
class Game
{
    sf::RenderWindow window;                          // Main game window
    std::vector<std::shared_ptr<Element>> elements;   // All available elements
    std::vector<std::shared_ptr<GameObject>> objects; // Active game objects in the world
    std::map<std::string, sf::Texture> textures;      // Loaded textures mapped by element name
    CombinationRegistry registry;                     // Handles element combination logic
    ElementBook book;                                 // Element encyclopedia
    sf::Texture trashTex;                             // Trash bin texture
    sf::Sprite trashBin;                              // Trash bin sprite for deleting objects
    std::shared_ptr<GameObject> draggingObject;       // Currently dragged object (if any)
    sf::Sprite invalidMark;                           // Red X shown for invalid combinations
    float invalidMarkTime;                            // When to stop showing invalid mark
    sf::Vector2f invalidMarkPos;                      // Position of invalid mark
    sf::Font font;                                    // Font for UI text
    sf::Clock clock;                                  // Game timer
    const int maxObjects = 50;                        // Maximum objects allowed in world
    float sidebarScroll = 0.0f;                       // Scroll offset for right sidebar
    const float scrollSpeed = 30.0f;                  // Pixels per scroll step

public:
    Game() : window(sf::VideoMode(800, 600), "Little Alchemist"), book(textures), invalidMarkTime(0)
    {
        window.setFramerateLimit(60); // Limit to 60 FPS

        // Load font for UI text
        if (!font.loadFromFile("fonts/Pixel Game.otf"))
        {
            std::cerr << "Failed to load font from fonts/Pixel Game.otf, using fallback fonts/arial.ttf\n";
            font.loadFromFile("fonts/arial.ttf");
        }

        // Define texture file paths for each element
        std::map<std::string, std::string> texturePaths = {
            // Basic Elements
            {"Fire", "assets/fire.png"},
            {"Water", "assets/water.png"},
            {"Earth", "assets/earth.png"},
            {"Air", "assets/air.png"},

            // Basic Combinations (2 basic elements)
            {"Steam", "assets/steam.png"}, // Fire + Water
            {"Lava", "assets/lava.png"},   // Fire + Earth
            {"Smoke", "assets/smoke.png"}, // Fire + Air
            {"Mud", "assets/mud.png"},     // Water + Earth
            {"Mist", "assets/mist.png"},   // Water + Air
            {"Dust", "assets/dust.png"},   // Earth + Air

            // Duplicate Element Combinations
            {"Energy", "assets/energy.png"},     // Fire + Fire
            {"Ocean", "assets/ocean.png"},       // Water + Water
            {"Mountain", "assets/mountain.png"}, // Earth + Earth
            {"Wind", "assets/wind.png"},         // Air + Air

            // Advanced Combinations
            {"Cloud", "assets/cloud.png"},         // Steam + Air
            {"Rain", "assets/rain.png"},           // Cloud + Water
            {"Plant", "assets/plant.png"},         // Water + Earth + Energy
            {"Stone", "assets/stone.png"},         // Lava + Air
            {"Volcano", "assets/volcano.png"},     // Lava + Mountain
            {"Lightning", "assets/lightning.png"}, // Energy + Air
            {"Ice", "assets/ice.png"},             // Water + Wind
            {"Sand", "assets/sand.png"},           // Stone + Wind
            {"Swamp", "assets/swamp.png"},         // Mud + Plant
            {"Forest", "assets/forest.png"},       // Plant + Plant
            {"Desert", "assets/desert.png"},       // Sand + Sand
            {"Life", "assets/life.png"}            // Energy + Plant
        };

        // Load all element textures
        for (const auto &pair : texturePaths)
        {
            sf::Texture tex;
            if (!tex.loadFromFile(pair.second))
            {
                std::cerr << "Failed to load texture: " << pair.second << "\n";
                // Create fallback magenta square if texture loading fails
                sf::Image img;
                img.create(50, 50, sf::Color::Magenta);
                tex.loadFromImage(img);
            }
            textures[pair.first] = tex;
        }

        // Initialize game elements (4 basic + 22 discoverable = 26 total)
        // Basic Elements (discovered = true)
        elements.push_back(std::make_shared<Element>("Fire", "A blazing flame", true));
        elements.push_back(std::make_shared<Element>("Water", "Crystal clear liquid", true));
        elements.push_back(std::make_shared<Element>("Earth", "Rich brown soil", true));
        elements.push_back(std::make_shared<Element>("Air", "Invisible breeze", true));

        // Basic Combinations (discovered = false)
        elements.push_back(std::make_shared<Element>("Steam", "Hot water vapor", false));
        elements.push_back(std::make_shared<Element>("Lava", "Molten rock and fire", false));
        elements.push_back(std::make_shared<Element>("Smoke", "Cloudy haze", false));
        elements.push_back(std::make_shared<Element>("Mud", "Wet and sticky earth", false));
        elements.push_back(std::make_shared<Element>("Mist", "Gentle water vapor", false));
        elements.push_back(std::make_shared<Element>("Dust", "Fine particles in air", false));

        // Duplicate Element Combinations
        elements.push_back(std::make_shared<Element>("Energy", "Pure concentrated power", false));
        elements.push_back(std::make_shared<Element>("Ocean", "Vast body of water", false));
        elements.push_back(std::make_shared<Element>("Mountain", "Towering earthen peak", false));
        elements.push_back(std::make_shared<Element>("Wind", "Strong moving air", false));

        // Advanced Combinations
        elements.push_back(std::make_shared<Element>("Cloud", "Fluffy sky formation", false));
        elements.push_back(std::make_shared<Element>("Rain", "Falling water droplets", false));
        elements.push_back(std::make_shared<Element>("Plant", "Green growing life", false));
        elements.push_back(std::make_shared<Element>("Stone", "Hard solid rock", false));
        elements.push_back(std::make_shared<Element>("Volcano", "Explosive mountain", false));
        elements.push_back(std::make_shared<Element>("Lightning", "Electric bolt", false));
        elements.push_back(std::make_shared<Element>("Ice", "Frozen water crystal", false));
        elements.push_back(std::make_shared<Element>("Sand", "Tiny rock particles", false));
        elements.push_back(std::make_shared<Element>("Swamp", "Muddy wetland", false));
        elements.push_back(std::make_shared<Element>("Forest", "Dense tree collection", false));
        elements.push_back(std::make_shared<Element>("Desert", "Vast sandy wasteland", false));
        elements.push_back(std::make_shared<Element>("Life", "The essence of living things", false));

        // Add all elements to the book
        for (auto &elem : elements)
        {
            book.addElement(elem);
        }

        // Initialize trash bin sprite
        if (!trashTex.loadFromFile("assets/trashbin.png"))
        {
            std::cerr << "Failed to load trash icon: assets/trashbin.png\n";
            // Create fallback red square if texture loading fails
            sf::Image img;
            img.create(30, 30, sf::Color::Red);
            trashTex.loadFromImage(img);
        }
        trashBin.setTexture(trashTex);
        trashBin.setPosition(10, window.getSize().y - 74);                             // Bottom-left corner
        trashBin.setScale(64.0f / trashTex.getSize().x, 64.0f / trashTex.getSize().y); // Scale to 64x64

        // Initialize invalid combination marker (red X)
        invalidMark.setTexture(book.getCrossTexture());
        invalidMark.setScale(24.0f / book.getCrossTexture().getSize().x, 24.0f / book.getCrossTexture().getSize().y);
        invalidMark.setColor(sf::Color::Red);
    }

    /**
     * Main game loop - runs until window is closed
     */
    void run()
    {
        while (window.isOpen())
        {
            handleEvents();
            update(clock.getElapsedTime().asSeconds());
            draw();
        }
    }

private:
    /**
     * Handle all input events (mouse clicks, window close, etc.)
     */
    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Handle window close button
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // Handle scroll wheel for right sidebar
            if (event.type == sf::Event::MouseWheelScrolled)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseWheelScroll.x, event.mouseWheelScroll.y));

                // Check if mouse is over right sidebar
                if (mousePos.x > window.getSize().x - 100 && !book.isBookOpen())
                {
                    sidebarScroll -= event.mouseWheelScroll.delta * scrollSpeed;

                    // Get count of discovered elements
                    int discoveredCount = 0;
                    for (auto &elem : elements)
                    {
                        if (elem->discovered)
                            discoveredCount++;
                    }

                    // Clamp scroll bounds
                    float maxScroll = std::max(0.0f, discoveredCount * 30.0f - window.getSize().y + 50);
                    sidebarScroll = std::max(0.0f, std::min(sidebarScroll, maxScroll));
                }
            }

            // Let book handle its input first
            book.handleInput(event, window);

            // Skip game input handling if book is open
            if (book.isBookOpen())
            {
                continue;
            }

            // Handle mouse button press
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

                // Check if clicking on element buttons in right sidebar
                int discoveredIndex = 0;
                for (size_t i = 0; i < elements.size(); ++i)
                {
                    if (!elements[i]->discovered)
                        continue;

                    sf::RectangleShape clickArea(sf::Vector2f(100, 30));
                    clickArea.setPosition(705, 10 + discoveredIndex * 30 - sidebarScroll);

                    // Only check if button is visible
                    if (clickArea.getPosition().y >= -30 && clickArea.getPosition().y <= window.getSize().y)
                    {
                        if (clickArea.getGlobalBounds().contains(mousePos) && objects.size() < maxObjects)
                        {
                            std::string spritePath = "assets/" + elements[i]->name + ".png";
                            auto obj = std::make_shared<GameObject>(elements[i], textures[elements[i]->name],
                                                                    spritePath, sf::Vector2f(400, 300), clock.getElapsedTime().asSeconds());
                            objects.push_back(obj);
                            elements[i]->creationCount++;
                            break;
                        }
                    }
                    discoveredIndex++;
                }

                // Check if clicking on existing objects to start dragging
                for (auto &obj : objects)
                {
                    if (obj->sprite.getGlobalBounds().contains(mousePos))
                    {
                        obj->isDragging = true;
                        draggingObject = obj;
                        break;
                    }
                }
            }

            // Handle mouse button release (end dragging)
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                if (draggingObject)
                {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

                    // Check if dropping object in trash bin
                    if (draggingObject->sprite.getGlobalBounds().intersects(trashBin.getGlobalBounds()))
                    {
                        // Remove object from world
                        objects.erase(std::remove_if(objects.begin(), objects.end(),
                                                     [&](auto &o)
                                                     { return o == draggingObject; }),
                                      objects.end());
                    }
                    else
                    {
                        // Check for combinations with other objects
                        checkCollisions(draggingObject, clock.getElapsedTime().asSeconds());
                    }

                    // Stop dragging
                    draggingObject->isDragging = false;
                    draggingObject = nullptr;
                }
            }

            // Handle mouse movement while dragging
            if (event.type == sf::Event::MouseMoved && draggingObject)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
                draggingObject->sprite.setPosition(mousePos - sf::Vector2f(25, 25)); // Center sprite on mouse
            }
        }
    }

    /**
     * Check for collisions between dragged object and other objects
     * Handle element combinations and invalid combination feedback
     */
    void checkCollisions(std::shared_ptr<GameObject> dragged, float time)
    {
        auto tempDragged = dragged;
        std::vector<std::shared_ptr<GameObject>> toRemove;

        // Check collision with each other object
        for (auto it = objects.begin(); it != objects.end(); ++it)
        {
            auto &other = *it;
            if (other == tempDragged || other->isDragging)
                continue; // Skip self and other dragging objects

            // Check if sprites overlap
            if (tempDragged->sprite.getGlobalBounds().intersects(other->sprite.getGlobalBounds()))
            {
                // Try to combine the elements
                std::string result = registry.getResult(tempDragged->element->name, other->element->name);

                if (!result.empty())
                {
                    // Valid combination - remove both objects and create result
                    toRemove.push_back(tempDragged);
                    toRemove.push_back(other);

                    // Find the result element and create new object
                    for (auto &elem : elements)
                    {
                        if (elem->name == result)
                        {
                            elem->discovered = true; // Discover the new element
                            elem->creationCount++;
                            std::string spritePath = "assets/" + result + ".png";
                            auto newObj = std::make_shared<GameObject>(elem, textures[result],
                                                                       spritePath,
                                                                       (tempDragged->sprite.getPosition() + other->sprite.getPosition()) / 2.0f,
                                                                       time);
                            objects.push_back(newObj);
                            break;
                        }
                    }
                    break;
                }
                else
                {
                    // Invalid combination - show red X and make other object semi-transparent
                    invalidMarkPos = (tempDragged->sprite.getPosition() + other->sprite.getPosition()) / 2.0f;
                    invalidMarkTime = time + 1.0f;                         // Show for 1 second
                    other->sprite.setColor(sf::Color(255, 255, 255, 128)); // Semi-transparent
                }
            }
        }

        // Remove objects that were used in combinations
        if (!toRemove.empty())
        {
            objects.erase(
                std::remove_if(objects.begin(), objects.end(),
                               [&](const auto &obj)
                               { return std::find(toRemove.begin(), toRemove.end(), obj) != toRemove.end(); }),
                objects.end());
        }
    }

    /**
     * Update game state each frame
     */
    void update(float time)
    {
        // Remove oldest objects if over the limit
        while (objects.size() > maxObjects)
        {
            auto oldest = std::min_element(objects.begin(), objects.end(),
                                           [](auto &a, auto &b)
                                           { return a->creationTime < b->creationTime; });
            objects.erase(oldest);
        }

        // Reset all object colors to white (remove semi-transparency from failed combinations)
        for (auto &obj : objects)
        {
            obj->sprite.setColor(sf::Color::White);
        }
    }

    /**
     * Render all game elements to the screen
     */
    void draw()
    {
        window.clear(sf::Color(255, 255, 255)); // Background

        sf::Vector2u windowSize = window.getSize();
        float sidebarWidth = 100.0f;

        // Draw main sandbox area (left side)
        sf::RectangleShape sandbox(sf::Vector2f(windowSize.x - sidebarWidth, windowSize.y));
        sandbox.setPosition(0, 0);
        sandbox.setFillColor(sf::Color(243, 124, 84)); // Main sandbox color
        window.draw(sandbox);

        // Draw right sidebar for element buttons
        sf::RectangleShape rightTab(sf::Vector2f(sidebarWidth, windowSize.y));
        rightTab.setPosition(windowSize.x - sidebarWidth, 0);
        rightTab.setFillColor(sf::Color(255, 194, 77)); // Light gray
        window.draw(rightTab);

        // Draw discovered element buttons in right sidebar with scrolling
        int discoveredIndex = 0;
        for (size_t i = 0; i < elements.size(); ++i)
        {
            if (elements[i]->discovered)
            {
                float yPos = 10 + discoveredIndex * 30 - sidebarScroll;

                // Only draw if visible
                if (yPos >= -30 && yPos <= windowSize.y)
                {
                    // Draw element icon
                    sf::Sprite icon;
                    auto it = textures.find(elements[i]->name);
                    if (it != textures.end())
                    {
                        icon.setTexture(it->second);
                        icon.setScale(20.0f / it->second.getSize().x, 20.0f / it->second.getSize().y);
                        icon.setPosition(705, yPos);
                        window.draw(icon);
                    }

                    // Draw element name
                    sf::Text text(elements[i]->name, font, 20);
                    text.setPosition(730, yPos);
                    text.setFillColor(sf::Color::Black);
                    window.draw(text);
                }
                discoveredIndex++;
            }
        }

        // Draw all game objects (except currently dragged one)
        for (auto &obj : objects)
        {
            if (!obj->isDragging)
                window.draw(obj->sprite);
        }

        // Draw dragged object on top of everything else
        if (draggingObject)
            window.draw(draggingObject->sprite);

        // Draw trash bin
        window.draw(trashBin);

        // Draw invalid combination marker (red X) if needed
        if (invalidMarkTime > clock.getElapsedTime().asSeconds())
        {
            invalidMark.setPosition(invalidMarkPos);
            window.draw(invalidMark);
        }

        // Draw the element book interface
        book.draw(window, clock.getElapsedTime().asSeconds());

        // Present the frame to the screen
        window.display();
    }  
};

/**
 * Program entry point - creates and runs the game
 */
int main()
{
    Game game;
    game.run();
    return 0;
}