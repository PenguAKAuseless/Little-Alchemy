# Little Alchemist - Adding New Elements Guide

This guide explains how to add new elements to the Little Alchemist game, including both basic elements (available from start) and discoverable elements (created through combinations).

## Table of Contents
- [Overview](#overview)
- [Adding Basic Elements](#adding-basic-elements)
- [Adding Discoverable Elements](#adding-discoverable-elements)
- [Asset Requirements](#asset-requirements)
- [Complete Example](#complete-example)
- [Testing Your Changes](#testing-your-changes)
- [Troubleshooting](#troubleshooting)

## Overview

Elements in Little Alchemist come in two types:
- **Basic Elements**: Available from the start (Fire, Air)
- **Discoverable Elements**: Created by combining other elements (Smoke = Fire + Air)

## Adding Basic Elements

Basic elements are immediately available to players and appear in the right sidebar from game start.

### Step 1: Add the Element Asset

1. Create or find a PNG image for your element (recommended size: 50x50 to 100x100 pixels)
2. Save it in the `assets/` folder with a descriptive name
   ```
   assets/water.png
   assets/earth.png
   ```

### Step 2: Update the Texture Paths

In `main.cpp`, locate the `texturePaths` map in the `Game` constructor:

```cpp
std::map<std::string, std::string> texturePaths = {
    {"Fire", "assets/fire.png"},
    {"Air", "assets/air.png"},
    {"Smoke", "assets/smoke.png"},
    // Add your new basic elements here
    {"Water", "assets/water.png"},
    {"Earth", "assets/earth.png"}
};
```

### Step 3: Add Element Definition

Add your element to the elements vector in the `Game` constructor. Set `discovered` to `true` for basic elements:

```cpp
// Initialize game elements
elements.push_back(std::make_shared<Element>("Fire", "A blazing flame", true));
elements.push_back(std::make_shared<Element>("Air", "Invisible breeze", true));
elements.push_back(std::make_shared<Element>("Smoke", "Cloudy haze", false));
// Add your new basic elements
elements.push_back(std::make_shared<Element>("Water", "Crystal clear liquid", true));
elements.push_back(std::make_shared<Element>("Earth", "Rich brown soil", true));
```

**Element Constructor Parameters:**
- `name`: Element name (must match texture map key)
- `description`: Descriptive text shown in the element book
- `discovered`: `true` for basic elements, `false` for discoverable ones

## Adding Discoverable Elements

Discoverable elements are created through combinations and must be unlocked by players.

### Step 1: Add the Element Asset

Same as basic elements - add your PNG file to the `assets/` folder:
```
assets/mud.png
assets/steam.png
```

### Step 2: Update Texture Paths

Add to the `texturePaths` map:
```cpp
std::map<std::string, std::string> texturePaths = {
    {"Fire", "assets/fire.png"},
    {"Air", "assets/air.png"},
    {"Smoke", "assets/smoke.png"},
    {"Water", "assets/water.png"},
    {"Earth", "assets/earth.png"},
    // Add discoverable elements
    {"Mud", "assets/mud.png"},
    {"Steam", "assets/steam.png"}
};
```

### Step 3: Add Element Definition

Add to elements vector with `discovered` set to `false`:
```cpp
elements.push_back(std::make_shared<Element>("Fire", "A blazing flame", true));
elements.push_back(std::make_shared<Element>("Air", "Invisible breeze", true));
elements.push_back(std::make_shared<Element>("Water", "Crystal clear liquid", true));
elements.push_back(std::make_shared<Element>("Earth", "Rich brown soil", true));
elements.push_back(std::make_shared<Element>("Smoke", "Cloudy haze", false));
// Add your discoverable elements
elements.push_back(std::make_shared<Element>("Mud", "Wet and sticky earth", false));
elements.push_back(std::make_shared<Element>("Steam", "Hot water vapor", false));
```

### Step 4: Add Combination Formulas

In the `CombinationRegistry` constructor, add the recipes for creating your new elements:

```cpp
CombinationRegistry()
{
    // Existing combinations
    combinations[{"Fire", "Air"}] = "Smoke";
    combinations[{"Air", "Fire"}] = "Smoke";
    
    // Add new combinations (remember both orders!)
    combinations[{"Water", "Earth"}] = "Mud";
    combinations[{"Earth", "Water"}] = "Mud";
    
    combinations[{"Fire", "Water"}] = "Steam";
    combinations[{"Water", "Fire"}] = "Steam";
}
```

**Important:** Always add both orders of the combination (A+B and B+A) since players can drag elements in either order.

### Step 5: Update Formula Display

In the `ElementBook::draw()` method, update the formula display logic to show your new combinations:

```cpp
if (elem->discovered)
{
    std::string formula;
    if (elem->name == "Smoke") {
        formula = "Fire + Air";
    } else if (elem->name == "Mud") {
        formula = "Water + Earth";
    } else if (elem->name == "Steam") {
        formula = "Fire + Water";
    } else {
        formula = "Basic Element";
    }
    
    details.setString("Name: " + elem->name + "\nCreated: " + std::to_string(elem->creationCount) +
                      "\nDescription: " + elem->description + "\nFormula: " + formula);
}
```

## Asset Requirements

### Image Specifications
- **Format**: PNG (supports transparency)
- **Size**: 50x50 to 100x100 pixels recommended
- **Style**: Should match existing art style
- **Background**: Transparent or matching game aesthetic

### File Naming Convention
- Use lowercase names matching the element name
- No spaces (use underscores if needed)
- Examples: `fire.png`, `water.png`, `lava_rock.png`

### File Location
All element images must be placed in the `assets/` folder:
```
assets/
├── fire.png
├── air.png
├── water.png
├── earth.png
├── smoke.png
├── mud.png
├── steam.png
└── ... (other assets)
```

## Complete Example

Let's add "Lava" as a new discoverable element (Fire + Earth):

### 1. Add Asset
Save `lava.png` to `assets/lava.png`

### 2. Update Texture Paths
```cpp
std::map<std::string, std::string> texturePaths = {
    {"Fire", "assets/fire.png"},
    {"Air", "assets/air.png"},
    {"Water", "assets/water.png"},
    {"Earth", "assets/earth.png"},
    {"Smoke", "assets/smoke.png"},
    {"Lava", "assets/lava.png"}  // Add this line
};
```

### 3. Add Element Definition
```cpp
elements.push_back(std::make_shared<Element>("Fire", "A blazing flame", true));
elements.push_back(std::make_shared<Element>("Air", "Invisible breeze", true));
elements.push_back(std::make_shared<Element>("Water", "Crystal clear liquid", true));
elements.push_back(std::make_shared<Element>("Earth", "Rich brown soil", true));
elements.push_back(std::make_shared<Element>("Smoke", "Cloudy haze", false));
elements.push_back(std::make_shared<Element>("Lava", "Molten rock and fire", false));  // Add this line
```

### 4. Add Combination Recipe
```cpp
CombinationRegistry()
{
    combinations[{"Fire", "Air"}] = "Smoke";
    combinations[{"Air", "Fire"}] = "Smoke";
    // Add both directions for Lava
    combinations[{"Fire", "Earth"}] = "Lava";
    combinations[{"Earth", "Fire"}] = "Lava";
}
```

### 5. Update Formula Display
```cpp
std::string formula;
if (elem->name == "Smoke") {
    formula = "Fire + Air";
} else if (elem->name == "Lava") {
    formula = "Fire + Earth";  // Add this case
} else {
    formula = "Basic Element";
}
```

## Testing Your Changes

### Compilation
```bash
g++ -c main.cpp -o main.o -std=c++17
g++ main.o -o game -lsfml-graphics -lsfml-window -lsfml-system
```

### Testing Checklist
1. **Basic Elements**: Verify new basic elements appear in the right sidebar
2. **Assets**: Check that images load correctly (no magenta placeholders)
3. **Combinations**: Test that combinations work in both directions
4. **Element Book**: Verify elements appear in the book with correct descriptions
5. **Formula Display**: Check that formulas show correctly in element details

## Troubleshooting

### Common Issues

**Element not appearing in sidebar:**
- Check that `discovered` is set to `true` for basic elements
- Verify element name matches exactly between texture map and element definition

**Magenta placeholder instead of image:**
- Verify PNG file exists in `assets/` folder
- Check file path in `texturePaths` map
- Ensure file name matches exactly (case-sensitive)

**Combination not working:**
- Make sure you added both directions (A+B and B+A)
- Check that element names in combination match exactly
- Verify the result element is defined in the elements vector

**Element not showing in book:**
- Ensure `book.addElement(elem)` is called for the element
- Check that the element is added to the elements vector

**Formula not displaying correctly:**
- Update the formula display logic in `ElementBook::draw()`
- Make sure the condition matches the element name exactly

### Debug Tips

1. **Console Output**: Check terminal for error messages about missing files
2. **Texture Loading**: Failed texture loads will print error messages
3. **Case Sensitivity**: File names and element names are case-sensitive
4. **Order Matters**: Elements should be added in a logical order (basic first, then discoverable)

## Adding More Complex Recipes

For elements requiring multiple steps:

```cpp
// Example: Cloud = Air + Steam (but Steam = Fire + Water)
// Player must first create Steam, then combine with Air

combinations[{"Air", "Steam"}] = "Cloud";
combinations[{"Steam", "Air"}] = "Cloud";
```

This creates discovery chains where players must experiment to find all combinations!

## Next Steps

Once you're comfortable adding elements, consider:
- Adding sound effects for combinations
- Creating element categories
- Adding particle effects
- Implementing more complex combination rules
- Adding achievements for discovering elements

Happy element crafting! 🧪✨