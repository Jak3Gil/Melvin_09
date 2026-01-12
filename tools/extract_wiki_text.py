#!/usr/bin/env python3
"""
Extract plain text from Wikipedia XML dump
Outputs clean text suitable for training
"""

import sys
import bz2
import re
from xml.etree import ElementTree as ET

def clean_text(text):
    """Clean Wikipedia markup from text"""
    if not text:
        return ""
    
    # Remove templates {{...}}
    text = re.sub(r'\{\{[^}]*\}\}', '', text)
    
    # Remove links [[...]]
    text = re.sub(r'\[\[([^|\]]*\|)?([^\]]*)\]\]', r'\2', text)
    
    # Remove references <ref>...</ref>
    text = re.sub(r'<ref[^>]*>.*?</ref>', '', text, flags=re.DOTALL)
    text = re.sub(r'<ref[^>]*\/>', '', text)
    
    # Remove HTML tags
    text = re.sub(r'<[^>]+>', '', text)
    
    # Remove wiki markup
    text = re.sub(r"'''", '', text)  # Bold
    text = re.sub(r"''", '', text)   # Italic
    text = re.sub(r'^[*#:;]+', '', text, flags=re.MULTILINE)  # Lists
    text = re.sub(r'^=+\s*(.+?)\s*=+$', r'\1', text, flags=re.MULTILINE)  # Headers
    
    # Remove URLs
    text = re.sub(r'https?://\S+', '', text)
    
    # Clean up whitespace
    text = re.sub(r'\n\n+', '\n\n', text)
    text = re.sub(r' +', ' ', text)
    
    return text.strip()

def extract_articles(xml_file, output_file, max_articles=None):
    """Extract articles from Wikipedia XML dump"""
    
    print(f"Extracting text from {xml_file}...")
    print(f"Output: {output_file}")
    
    # Open compressed file
    if xml_file.endswith('.bz2'):
        f = bz2.open(xml_file, 'rt', encoding='utf-8')
    else:
        f = open(xml_file, 'r', encoding='utf-8')
    
    out = open(output_file, 'w', encoding='utf-8')
    
    article_count = 0
    text_size = 0
    
    # Parse XML incrementally
    context = ET.iterparse(f, events=('start', 'end'))
    context = iter(context)
    
    _, root = next(context)
    
    current_title = None
    current_text = None
    in_page = False
    
    for event, elem in context:
        tag = elem.tag.split('}')[-1]  # Remove namespace
        
        if event == 'start' and tag == 'page':
            in_page = True
            current_title = None
            current_text = None
        
        elif event == 'end' and tag == 'title' and in_page:
            current_title = elem.text
        
        elif event == 'end' and tag == 'text' and in_page:
            current_text = elem.text
        
        elif event == 'end' and tag == 'page':
            if current_title and current_text:
                # Skip special pages
                if not current_title.startswith(('Wikipedia:', 'Template:', 'Category:', 'File:', 'Help:')):
                    # Clean text
                    clean = clean_text(current_text)
                    
                    if len(clean) > 100:  # Skip very short articles
                        # Write title and text
                        out.write(f"{current_title}\n")
                        out.write(f"{clean}\n\n")
                        
                        article_count += 1
                        text_size += len(clean)
                        
                        if article_count % 100 == 0:
                            mb = text_size / (1024 * 1024)
                            print(f"\r  Extracted {article_count} articles ({mb:.1f} MB)", end='', flush=True)
                        
                        if max_articles and article_count >= max_articles:
                            break
            
            in_page = False
            elem.clear()
            root.clear()
    
    f.close()
    out.close()
    
    mb = text_size / (1024 * 1024)
    print(f"\n\nExtracted {article_count} articles ({mb:.1f} MB)")
    print(f"Output saved to: {output_file}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 extract_wiki_text.py <wiki.xml.bz2> [output.txt] [max_articles]")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'wiki_text.txt'
    max_articles = int(sys.argv[3]) if len(sys.argv) > 3 else None
    
    extract_articles(input_file, output_file, max_articles)
