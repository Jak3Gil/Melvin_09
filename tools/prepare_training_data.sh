#!/bin/bash
#
# Prepare Training Data for Melvin
# Downloads and prepares ~1GB of text data from various sources
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="$SCRIPT_DIR/../training_data"

echo "╔════════════════════════════════════════════════════════════╗"
echo "║         MELVIN TRAINING DATA PREPARATION                   ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Data directory: $DATA_DIR"
echo ""

# Create data directory
mkdir -p "$DATA_DIR"
cd "$DATA_DIR"

# Function to download and extract
download_and_extract() {
    local url=$1
    local filename=$2
    local extract_cmd=$3
    
    if [ ! -f "$filename" ]; then
        echo "Downloading $filename..."
        curl -L -o "$filename" "$url"
        echo "✓ Downloaded $filename"
    else
        echo "✓ $filename already exists"
    fi
    
    if [ -n "$extract_cmd" ]; then
        echo "Extracting $filename..."
        eval "$extract_cmd"
        echo "✓ Extracted $filename"
    fi
}

echo "=== Downloading Text Corpora ==="
echo ""

# 1. Project Gutenberg books (~100MB)
echo "1. Project Gutenberg Books"
mkdir -p gutenberg
cd gutenberg

# Download a few classic books
books=(
    "https://www.gutenberg.org/cache/epub/1342/pg1342.txt:pride_and_prejudice.txt"
    "https://www.gutenberg.org/cache/epub/84/pg84.txt:frankenstein.txt"
    "https://www.gutenberg.org/cache/epub/1661/pg1661.txt:sherlock_holmes.txt"
    "https://www.gutenberg.org/cache/epub/11/pg11.txt:alice_wonderland.txt"
    "https://www.gutenberg.org/cache/epub/98/pg98.txt:tale_two_cities.txt"
    "https://www.gutenberg.org/cache/epub/1080/pg1080.txt:modest_proposal.txt"
    "https://www.gutenberg.org/cache/epub/2701/pg2701.txt:moby_dick.txt"
    "https://www.gutenberg.org/cache/epub/1952/pg1952.txt:yellow_wallpaper.txt"
    "https://www.gutenberg.org/cache/epub/16/pg16.txt:peter_pan.txt"
    "https://www.gutenberg.org/cache/epub/174/pg174.txt:dorian_gray.txt"
)

for book in "${books[@]}"; do
    url="${book%%:*}"
    filename="${book##*:}"
    if [ ! -f "$filename" ]; then
        echo "  Downloading $filename..."
        curl -s -L -o "$filename" "$url" || echo "  Warning: Failed to download $filename"
    fi
done

cd ..
echo "✓ Gutenberg books ready"
echo ""

# 2. Wikipedia sample
echo "2. Wikipedia Sample"
mkdir -p wikipedia
cd wikipedia

# Download Wikipedia simple articles dump (smaller, ~200MB compressed)
if [ ! -f "simplewiki-latest-pages-articles.xml.bz2" ]; then
    echo "  Downloading Wikipedia simple articles..."
    echo "  (This may take a while - ~200MB compressed)"
    curl -L -o "simplewiki-latest-pages-articles.xml.bz2" \
        "https://dumps.wikimedia.org/simplewiki/latest/simplewiki-latest-pages-articles.xml.bz2" || \
        echo "  Warning: Wikipedia download failed (optional)"
fi

cd ..
echo "✓ Wikipedia data ready"
echo ""

# 3. News articles (if available)
echo "3. News Articles"
mkdir -p news
cd news

# Create sample news articles
cat > tech_news.txt << 'EOF'
Artificial Intelligence Advances in 2026

Researchers have made significant breakthroughs in artificial intelligence this year. New neural network architectures are showing improved efficiency and reasoning capabilities. Machine learning models are becoming more interpretable and require less training data.

The field of natural language processing has seen particular progress, with models that can understand context and generate coherent responses. Computer vision systems are now more accurate at recognizing objects and scenes in complex environments.

Scientists are also exploring brain-inspired computing architectures that mimic biological neural networks. These systems show promise for energy-efficient AI that can learn continuously from experience.
EOF

cat > science_news.txt << 'EOF'
Climate Research Shows Promising Results

New studies in climate science reveal important findings about Earth's changing environment. Researchers are using advanced computer models to predict future climate patterns with greater accuracy.

Scientists have discovered new methods for carbon capture and storage. Renewable energy technologies continue to improve in efficiency and cost-effectiveness. Solar panels and wind turbines are becoming more widespread.

Marine biologists are studying ocean ecosystems to understand the impact of rising temperatures. Conservation efforts are helping to protect endangered species and preserve biodiversity.
EOF

cat > health_news.txt << 'EOF'
Medical Breakthroughs in Disease Treatment

Healthcare researchers have announced significant advances in treating various diseases. New therapies are showing promise in clinical trials for conditions that were previously difficult to treat.

Personalized medicine approaches are becoming more common, with treatments tailored to individual patients based on their genetic profiles. Diagnostic tools are becoming more accurate and accessible.

Public health initiatives are focusing on prevention and early detection. Vaccination programs continue to protect populations from infectious diseases. Mental health awareness and treatment options are expanding.
EOF

cd ..
echo "✓ News articles created"
echo ""

# 4. Educational content
echo "4. Educational Content"
mkdir -p education
cd education

cat > science_basics.txt << 'EOF'
Introduction to Physics

Physics is the natural science that studies matter, energy, and their interactions. It seeks to understand the fundamental laws that govern the universe.

Classical mechanics describes the motion of objects under the influence of forces. Newton's laws of motion form the foundation of this field. Objects at rest stay at rest unless acted upon by a force. Force equals mass times acceleration.

Thermodynamics deals with heat, temperature, and energy transfer. The first law states that energy is conserved. The second law introduces the concept of entropy, which tends to increase in isolated systems.

Electromagnetism explains electric and magnetic phenomena. Electric charges create electric fields. Moving charges create magnetic fields. Light is an electromagnetic wave that travels through space.

Quantum mechanics describes the behavior of matter at atomic and subatomic scales. Particles exhibit both wave and particle properties. The uncertainty principle limits what can be known simultaneously about a particle's position and momentum.
EOF

cat > biology_basics.txt << 'EOF'
Introduction to Biology

Biology is the study of living organisms and their interactions with each other and their environment. It encompasses many fields from molecular biology to ecology.

Cells are the basic units of life. All living things are made of cells. Cells contain genetic material in the form of DNA. DNA carries instructions for building and maintaining organisms.

Evolution explains how species change over time through natural selection. Organisms with advantageous traits are more likely to survive and reproduce. This process leads to adaptation and diversity.

Ecology studies how organisms interact with their environment. Ecosystems consist of living organisms and non-living components. Energy flows through ecosystems in food chains and food webs.

Genetics examines how traits are inherited from parents to offspring. Genes are segments of DNA that code for proteins. Mutations in genes can lead to variation in traits.
EOF

cat > history_basics.txt << 'EOF'
World History Overview

Human civilization has developed over thousands of years. Early humans were hunter-gatherers who eventually developed agriculture. The agricultural revolution allowed for permanent settlements and the growth of cities.

Ancient civilizations emerged in river valleys around the world. Mesopotamia, Egypt, India, and China developed complex societies with writing systems, laws, and organized governments.

The classical period saw the rise of empires like Rome and Greece. These civilizations made lasting contributions to philosophy, science, and art. Trade routes connected distant regions and facilitated cultural exchange.

The medieval period in Europe was characterized by feudalism and the influence of the Catholic Church. The Renaissance brought renewed interest in classical learning and artistic achievement.

The Industrial Revolution transformed economies and societies. New technologies changed how people worked and lived. Urbanization increased as people moved to cities for factory jobs.

The modern era has seen rapid technological advancement and globalization. Communication and transportation technologies have connected the world. International cooperation and conflict have shaped global politics.
EOF

cd ..
echo "✓ Educational content created"
echo ""

# 5. Conversational data
echo "5. Conversational Data"
mkdir -p conversations
cd conversations

cat > dialogues.txt << 'EOF'
Person A: Hello, how are you today?
Person B: I'm doing well, thank you for asking. How about you?
Person A: I'm great! I've been working on an interesting project.
Person B: That sounds exciting. What kind of project?
Person A: It's a machine learning system that learns from text data.
Person B: Fascinating! How does it work?
Person A: It builds a graph of concepts and their relationships.
Person B: That's a clever approach. Does it work well?
Person A: Yes, it's showing promising results so far.

Customer: I'd like to order a coffee, please.
Barista: Sure! What size would you like?
Customer: Medium, please.
Barista: Would you like any milk or sugar?
Customer: Just a little milk, no sugar.
Barista: Coming right up. That'll be three dollars.
Customer: Here you go. Thank you!
Barista: You're welcome. Have a great day!

Teacher: Today we'll learn about photosynthesis.
Student: What is photosynthesis?
Teacher: It's how plants make food using sunlight.
Student: Do all plants do photosynthesis?
Teacher: Most plants do, especially green plants.
Student: Why are plants green?
Teacher: They contain chlorophyll, which absorbs light.
Student: That's interesting! Thank you for explaining.
Teacher: You're welcome. Any other questions?
EOF

cd ..
echo "✓ Conversational data created"
echo ""

# 6. Check total size
echo "=== Data Summary ==="
echo ""
total_size=$(du -sh . | cut -f1)
echo "Total data size: $total_size"
echo ""

# Count text files
txt_count=$(find . -name "*.txt" | wc -l | tr -d ' ')
echo "Text files: $txt_count"
echo ""

echo "╔════════════════════════════════════════════════════════════╗"
echo "║              DATA PREPARATION COMPLETE                     ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Data is ready in: $DATA_DIR"
echo ""
echo "To train Melvin, run:"
echo "  cd $SCRIPT_DIR/.."
echo "  ./tools/train_large_corpus trained_brain.m training_data/"
echo ""
