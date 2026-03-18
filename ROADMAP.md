# Fremdriftsplan for nettside

Dette dokumentet beskriver videre trinn for utvikling og vedlikehold av nettstedet ditt.

## Nåværende status

- Eleventy-basert statisk generator er satt opp.
- Innholdsnavigasjon: `content/` inneholder markdown, pdfs, og layouts.
- `site/` inneholder genererte HTML-filer.
- `package.json` har `build` og `serve` skript.
- README beskriver hvordan bygge og serve prosjektet.

## Kortsiktige oppgaver (neste par dager)

1. **Legge til foreløpig innhold**
   - Opprett markdown-filer for hver forelesning, arbeidskrav og artikkel.
   - Oppsummer viktige rapporter i pensum (chalmers, bayne, osv.).
   - Sørg for at alle PDF-er er kopiert til `content/pdfs`.
2. **Teste side-navigasjon**
   - Opprett interne lenker i `index.md` for raske navigasjon.
   - Verifiser at `npm run serve` rendrer alt riktig.
3. **Publisering for testing**
   - Lag en `gh-pages` branch eller en Netlify/ Vercel deploy for å se siden live.
   - Del URL med medstudenter hvis ønskelig.

## Mellomliggende oppgaver (1–2 uker)

1. **Forbedre layout og stil**
   - Utvid `styles.css` med typografiske regler og fargevalg.
   - Legg til site-header/footer i layout.njk.
   - Vurder å bruke et CSS-rammeverk (f.eks. Bulma, Tailwind).
2. **Implementer søk/filter**
   - Generer en JSON-index over sider via `_data` og legg til Lunr.js i en skriptfil.
   - Legg inn søkefelt i layout og test respons.
3. **Metadata og tagging**
   - Legg til YAML-frontmatter felter: `date`, `tags`, `category`.
   - Opprett sider som lister etter tag eller kategori.

## Langsiktige mål

1. **Interaktiv innhold**
   - Legg til quiz- eller flashcard-komponenter (Vue/React via Eleventy).
   - Integrer med en backend for å logge studieresultater (valgfritt).
2. **Backup og versjonshistorikk**
   - Sett opp GitHub Actions som kjører bygge og deploy ved push.
   - Legg til rutine for sikkerhetskopiering av `content`.
3. **Skalerbar organisasjon**
   - Lag en admin-grensesnitt (enten statisk med Netlify CMS eller lignende).
   - Dokumenter hvordan andre kan bidra til innhold (fork og PR workflow).

## Vedlikehold

- Oppdater Eleventy, Node og avhengigheter regelmessig (`npm audit`, `npm update`).
- Rydd i gamle pdf-er og markdown-filer når de ikke lenger er nødvendige.
- Gjennomfør jevnlige gjennomganger av layout og tilgjengelighet.

> Notater: Dette roadmapet kan endres etter behov, men gir et strukturert utgangspunkt for fremtidig arbeid.
