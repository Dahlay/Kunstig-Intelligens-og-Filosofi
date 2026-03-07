# KI Studieplan Website

Denne enkle statiske nettsiden samler forelesningsnotater, artikler og pdf-er fra kurset ditt.

## Innholdsstruktur

- `content/` - rå markdown-filer for hver seksjon.
- `site/` - bygde HTML-filer og statiske ressurser.
- `scripts/build_site.py` - Python-skript som konverterer markdown til HTML.
- `site/pdfs/` - kopierte PDF-er for nedlasting.

## Brukerveiledning

> **Merk:** Skriptet `scripts/build_site.py` var den opprinnelige Python-baserte generatoren, men den er nå
erstattet av Eleventy. Det kan fjernes hvis ønskelig.

## Brukerveiledning

1. **Legg til innhold**:
   - Legg markdown-filer i `content/forelesninger`, `content/artikler`, `content/arbeidskrav` eller direkte i `content/`.
   - PDF-er legges i `content/pdfs` og kopieres automatisk.
   - Du kan beholde navngivning og organisere i undermapper.

2. **Bygg nettstedet**:
   ```bash
   npm run build
   ```
   HTML-filer genereres i `site/`.

3. **Se resultatet lokalt**:
   ```bash
   npm run serve
   ```
   Dette starter en lokal dev‑server (standardport 8080).

4. **Deploy**:
   - Push `site/`-innholdet til en webserver, eller bruk GitHub Pages/Netlify/Vercel ved å peke til denne mappen.

## Vedlikehold og utvidelse

- Filtrering og søk kan legges til ved å generere en JSON-index under `_data` og bruke JavaScript.
- Legg til flere Nunjucks‑layouts i `content/_includes` for avansert templating.
- Metadata håndteres via YAML-frontmatter i hver Markdown-fil.

Denne løsningen bruker Eleventy (et Node.js-bibliotek) og krever Node/npm, som installeres med nvm eller annen metode.

---

Dette er en minimal løsning uten Node eller andre avhengigheter. Python og markdown er alt som trengs.
