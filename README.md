# M5Stack Flipcard Language Learning App

A JSON-driven interactive flashcard application for M5Stack ESP32-S3 devices with E-paper displays. Features a sophisticated quad-mode interface for organized language learning with category-based navigation, multi-language support, and automatic power management.

## Features

### Multi-Mode Interface
- **Menu Mode**: Main entry point with Category, Random, and Option buttons
- **Category Mode**: Dynamic category selection with automatic card counting
- **Grid Mode**: 3×5 thumbnail view with category filtering and paging navigation
- **Flipcard Mode**: Detailed 3-part layout with language cycling and filtered navigation
- **Option Mode**: Language configuration and settings management
- **Random Mode**: No-duplicate randomized card selection within categories

### Advanced Navigation
- **Category-Based Filtering**: Complete filtering system across all navigation modes
- **Touch Interface**: Precise touch detection with 50ms polling and spacing-aware calculations
- **Circular Navigation**: Infinite loops in grid paging and card browsing within categories
- **Smart Back Navigation**: Proper mode transitions with filter persistence

### Language Learning Features
- **Multi-Language Content**: Support for any language combination (Chinese/English, Japanese/Indonesian, etc.)
- **3-Part Card Layout**: Big image for big text (400×150px), Small image for small text (400×80px), Main image (400×400px)
- **Dynamic Language Cycling**: Touch center area to cycle through enabled languages
- **Pronunciation Support**: Small images for phonetics, pinyin, or pronunciation guides
- **Visual Learning**: Large illustrations with language-specific text overlays

### Technical Features
- **JSON-Driven Architecture**: Fully data-driven with no hardcoded content
- **Configuration Management**: Persistent settings with live reload and validation
- **Power Management**: Automatic deep sleep after 5 minutes of inactivity with custom screensaver
- **EPaper Optimization**: White backgrounds with black borders for optimal display
- **Memory Efficiency**: Lazy loading with proper resource management
- **Scalable Design**: Add unlimited cards, categories, and languages via JSON only

## Hardware Requirements

- **Board**: ESP32-S3-DevKitC-1 with 16MB flash
- **Display**: E-paper display (540×960 resolution) via epdiy library
- **Storage**: SD card (SPI interface)
- **Touch**: Capacitive touch interface

### Pin Configuration
```
SD_CS: 47
SD_SCK: 39
SD_MOSI: 38
SD_MISO: 40
```

## Software Dependencies

- **PlatformIO**: Build system and framework
- **Arduino ESP32**: Core framework
- **M5Unified**: M5Stack hardware abstraction
- **ArduinoJson v7.0.4**: JSON parsing and manipulation
- **epdiy**: E-paper display driver (commit: d84d26ebebd780c4c9d4218d76fbe2727ee42b47)

## Installation & Setup

### 1. Hardware Setup
1. Connect E-paper display to ESP32-S3 via epdiy
2. Insert SD card with properly formatted content (see Data Structure below)
3. Ensure touch interface is properly connected

### 2. Software Setup
```bash
# Clone repository
git clone <repository-url>
cd flipcard-papers3

# Build and upload - see Development Notes section for detailed commands
```

### 3. SD Card Preparation
Copy the entire `flipcard/` directory to the root of your SD card. Ensure proper file structure as documented below. The `sd_card_content/flipcard/` folder in this repository contains sample data. For additional flipcard collections, extract the `collection-01.zip` file (included in the repository) and copy its contents to your SD card. 

## Data Structure & JSON Formats

### Folder Structure
```
/flipcard/                          # Root directory on SD card
├── config.json                     # Global configuration
├── index.json                      # Card index and metadata
├── any-folder-name/                # Individual card directory (name defined in index.json)
│   ├── card.json                   # Card-specific data
│   ├── main-image.png              # Main illustration (400×400px)
│   ├── thumbnail.png               # Thumbnail (140×140px)
│   ├── big-chinese.png             # Chinese text (400×150px)
│   ├── small-chinese.png           # Pinyin (400×80px)
│   ├── big-english.png             # English text (400×150px)
│   └── small-english.png           # Pronunciation (400×80px)
├── another-card-folder/            # Another card (flexible naming)
│   └── ...
├── Left.png                        # Navigation button (80×80px)
├── Right.png                       # Navigation button (80×80px)
├── Home.png                        # Navigation button (80×80px)
├── LeftGrey.png                    # Inactive nav button (80×80px)
├── RightGrey.png                   # Inactive nav button (80×80px)
├── empty-frame.png                 # Background for flipcard mode
└── screensaver/
    └── Thousand-Miles1.png         # Sleep mode image (540×960px)
```

**Note**: Folder names are completely flexible and defined in `index.json`. You can use any naming convention:
- `bicycle-card/`, `transport-001/`, `chinese-vocab-bike/`
- `中文单词/`, `folder_with_underscores/`, `CamelCaseFolder/`
- Any UTF-8 compatible folder name supported by your filesystem

### JSON Configuration Files

#### 1. Main Index (`/flipcard/index.json`)
```json
{
  "metadata": {
    "version": "2.0",
    "description": "Flipcard Index",
    "total_cards": 3
  },
  "cards": [
    {
      "id": "001",
      "folder": "bicycle-transport",
      "title": "Bicycle",
      "category": "transport",
      "thumbnail": "bicycle-thumb.png",
      "languages": ["chinese", "english"]
    }
  ],
  "categories": {
    "transport": {
      "name": "Transportation"
    }
  }
}
```


#### 2. Individual Card (`/flipcard/{folder-name}/card.json`)
```json
{
  "id": "001",
  "title": "Bicycle",
  "category": "transport",
  "main_image": "bicycle-illustration.png",
  "thumbnail": "bicycle-thumb.png",
  "languages": {
    "chinese": {
      "big_text": "自行车",
      "small_text": "zì xíng chē",
      "big_file": "bicycle-chinese-big.png",
      "small_file": "bicycle-chinese-small.png"
    },
    "english": {
      "big_text": "Bicycle",
      "small_text": "/ˈbaɪsɪkəl/",
      "big_file": "bicycle-english-big.png",
      "small_file": "bicycle-english-small.png"
    }
  }
}
```


#### 3. Configuration (`/flipcard/config.json`)
```json
{
  "metadata": {
    "version": "1.0",
    "description": "Flipcard system configuration"
  },
  "languages": {
    "default": "chinese",
    "supported": {
      "chinese": {
        "name": "中文",
        "english_name": "Chinese",
        "enabled": true
      },
      "english": {
        "name": "English", 
        "english_name": "English",
        "enabled": true
      }
    }
  },
  "navigation": {
    "auto_advance": false,
    "loop_cards": true,
    "touch_enabled": true
  }
}
```

## Image Requirements

### Required Dimensions
- **Big Images**: 400×150px (language-specific main content)
- **Small Images**: 400×80px (pronunciation/transliteration)
- **Main Images**: 400×400px (shared illustrations)
- **Thumbnails**: 140×140px (grid view previews)
- **Navigation Buttons**: 80×80px (Left.png, Right.png, Home.png)
- **Screensaver**: 540×960px (sleep mode display)

### File Naming Convention
- **Complete Flexibility**: All file names are defined in JSON - no hardcoded patterns
- **Language Images**: Any filename specified in card JSON `big_file` and `small_file` fields
- **Shared Images**: Any filename specified in card JSON `main_image` and `thumbnail` fields  
- **Navigation**: `Left.png`, `Right.png`, `Home.png`, `LeftGrey.png`, `RightGrey.png` (fixed names)
- **Background**: `empty-frame.png` (fixed name)

## Usage Guide

### Navigation Flow
```
Menu (Entry Point)
├── Category → Category Selection → Grid Mode → Flipcard Mode
├── Random → Category Selection → Flipcard Mode (skip grid)
└── Option → Language Settings / Configuration
```

### Touch Controls

- **Menu Mode**: Category/Random/Option buttons for main navigation
- **Category Mode**: Select any category or Home to return to menu
- **Grid Mode**: Touch thumbnails to view cards, Left/Right for paging, Home to return
- **Flipcard Mode**: Left/Right navigate cards, Center cycles languages, Home returns to grid
- **Option Mode**: Touch languages to set as default (marked with asterisk)

### Learning Modes

#### Structured Learning
1. **Menu → Category**: Choose specific topic (Transport, Technology, etc.)
2. **Grid View**: Browse visual thumbnails of filtered cards
3. **Flipcard Study**: Detailed study with language cycling
4. **Progress**: Systematic coverage of category content

#### Random Learning
1. **Menu → Random**: Choose category for random selection
2. **Direct Flipcard**: Jump straight to random card study
3. **No Duplicates**: Algorithm ensures variety within session
4. **Category Focus**: Random selection limited to chosen category

## Power Management

- **Auto Sleep**: Device sleeps after 5 minutes of inactivity
- **Custom Screensaver**: Displays `/flipcard/screensaver/Thousand-Miles1.png`
- **Wake on Touch**: Any touch input wakes the device
- **Battery Optimization**: Deep sleep mode significantly extends battery life

## Development Notes

### Build Commands
```bash
# Build project
pio run

# Upload to device  
pio run --target upload

# Monitor serial output
pio device monitor

# Clean build files
pio run --target clean
```

### Adding New Content

#### New Card
1. Create folder with any name: `/flipcard/your-folder-name/`
2. Add `card.json` with proper structure and file references
3. Add required images with names matching your JSON file references
4. Update `index.json` to include new card entry with correct folder name
5. Assign to existing or new category

#### New Category
1. Add card(s) with new category value
2. Update `categories` section in `index.json`
3. Cards automatically appear in category selection

#### New Language
1. Add language definition to `config.json`
2. Set `enabled: true` for new language
3. Add big/small image files for each card
4. Update card JSON files with new language data

## License

This project is designed for educational and personal language learning purposes. E-paper display integration via epdiy library following their respective licensing terms.

## Contributing

When adding new features or content:
1. Follow existing JSON structure and naming conventions
2. Maintain category-based organization
3. Test touch interface precision across all modes
4. Verify power management functionality
5. Ensure scalability for larger card collections

---
