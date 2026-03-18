#!/usr/bin/env python3
import os
import markdown

CONTENT_DIR = os.path.join(os.getcwd(), 'content')
OUTPUT_DIR = os.path.join(os.getcwd(), 'site')

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def convert_md_to_html(input_path, output_path):
    with open(input_path, 'r', encoding='utf-8') as f:
        text = f.read()
    html = markdown.markdown(text, extensions=['fenced_code', 'tables'])
    # simple template
    tmpl = f"""
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>{os.path.basename(input_path)}</title>
  <link rel="stylesheet" href="/styles.css">
</head>
<body>
<article>
{html}
</article>
</body>
</html>
"""
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(tmpl)


def build():
    # walk content directory
    for root, dirs, files in os.walk(CONTENT_DIR):
        for file in files:
            if file.endswith('.md'):
                rel_dir = os.path.relpath(root, CONTENT_DIR)
                out_dir = os.path.join(OUTPUT_DIR, rel_dir)
                ensure_dir(out_dir)
                input_path = os.path.join(root, file)
                name = os.path.splitext(file)[0] + '.html'
                out_path = os.path.join(out_dir, name)
                print(f'Converting {input_path} -> {out_path}')
                convert_md_to_html(input_path, out_path)
    # after converting markdown files, generate a simple PDF index
    pdf_dir = os.path.join(OUTPUT_DIR, 'pdfs')
    if os.path.isdir(pdf_dir):
        pdfs = sorted(os.listdir(pdf_dir))
        if pdfs:
            list_html = '<h2>PDF Resources</h2>\n<ul>\n'
            for pdf in pdfs:
                list_html += f'  <li><a href="/pdfs/{pdf}">{pdf}</a></li>\n'
            list_html += '</ul>\n'
            pdf_index_path = os.path.join(OUTPUT_DIR, 'pdfs.html')
            with open(pdf_index_path, 'w', encoding='utf-8') as f:
                f.write(f"""
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>PDF Resources</title>
  <link rel="stylesheet" href="/styles.css">
</head>
<body>
{list_html}
</body>
</html>
""")
            print(f'Generated PDF index at {pdf_index_path}')

if __name__ == '__main__':
    build()
