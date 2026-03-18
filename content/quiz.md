---
layout: layout.njk
title: "Quiz"
permalink: /quiz/index.html
---

<div class="lecture-header">
  <div class="lecture-number">Selvtest</div>
  <h1>Quiz</h1>
  <p class="lecturer">To moduser: åpne svar eller flervalg</p>
  <div class="lecture-meta">
    <span class="lecture-tag">36 åpne</span>
    <span class="lecture-tag">30 flervalg</span>
    <span class="lecture-tag">F1–F6</span>
  </div>
</div>

<div class="quiz-mode-tabs">
  <button class="qmode-btn active" id="tab-open" onclick="switchMode('open')">Åpne svar</button>
  <button class="qmode-btn" id="tab-mc" onclick="switchMode('mc')">Flervalg</button>
</div>

<div class="quiz-controls">
  <div class="quiz-filters" id="quizFilters"></div>
  <div class="quiz-score-bar">
    <span class="score-display" id="scoreDisplay">Riktig: 0 / Besvart: 0</span>
    <button class="reset-btn" onclick="resetQuiz()">Nullstill</button>
  </div>
</div>

<div id="quizContainer"></div>

{% raw %}
<script>
// ── OPEN QUESTIONS ────────────────────────────────────────────────────────────
var questions = [
  {f:"F1",q:"Hva er «hard problem of consciousness»?",
   a:"Spørsmålet om <em>hvorfor</em> det i det hele tatt er subjektiv opplevelse (qualia) ledsaget av hjerneaktivitet. «Easy problems» kan forklares av vitenskap; hard problem handler om selve opplevelsens eksistens."},
  {f:"F1", q:"Hva er qualia? Gi et eksempel.",
   a:"De kvalitative, subjektive egenskapene ved en opplevelse — rødhet av rødt, smertefullhet av smerte. Qualia er det som gjenstår etter at all fysisk og funksjonell beskrivelse er gitt."},
  {f:"F1", q:"Hva er forskjellen mellom fenomenal og aksess-bevissthet? (Block)",
   a:"<strong>Fenomenal:</strong> subjektiv opplevelse, qualia, «hva det er som å være X». <strong>Aksess:</strong> at informasjon er tilgjengelig for resonnement og rapportering. KI kan ha aksess-bevissthet uten fenomenal bevissthet."},
  {f:"F1", q:"Beskriv zombieargumentet og hva det er ment å vise. (Chalmers)",
   a:"En filosofisk zombie er identisk med et menneske i alle fysiske henseender, men uten subjektiv opplevelse. Hvis dette er konseptuelt mulig, er bevissthet ikke logisk bestemt av det fysiske — og dermed er fysikalisme feil."},
  {f:"F1", q:"Hva er «Mary's rom»-argumentet? (Jackson)",
   a:"Mary vet alt om fargesyn, men har bare sett sort/hvitt. Når hun ser rødt, lærer hun noe nytt — hva det er <em>som å se</em> rødt. Argument mot fysikalisme: det finnes kunnskap som ikke er fysisk."},
  {f:"F1", q:"Hva mener Nagel med at bevissthet har en «subjektiv karakter»?",
   a:"I «What Is It Like to Be a Bat?» argumenterer Nagel for at bevissthet har en uerstattelig førstepersons-karakter. Flaggermusens opplevelse er utilgjengelig for oss — den kan ikke fanges av objektiv tredjepersons vitenskap."},

  // F2 — Funksjonalisme
  {f:"F2", q:"Hva er funksjonalisme?",
   a:"Mentale tilstander defineres av funksjonelle roller — hva som forårsaker dem og hva de fører til — ikke av substansen de er laget av. Åpner i prinsippet for KI-bevissthet fordi substrat (karbon vs. silisium) er irrelevant."},
  {f:"F2", q:"Hva er multippel realiserbarhet?",
   a:"Den samme mentale tilstanden (f.eks. smerte) kan realiseres i ulike fysiske substrater — menneskehjernen, blekksprut, eller silisium. Viktig premiss for at KI i prinsippet kan ha mentale tilstander."},
  {f:"F2", q:"Hva er IIT og hva er φ (phi)?",
   a:"Integrated Information Theory (Tononi/Mørch): Bevissthet er identisk med integrert informasjon, målt som φ. Høy φ = rik bevissthet. Feeddforward-nettverk har lav φ → de fleste KI-modeller er trolig ikke bevisste ifølge IIT."},
  {f:"F2", q:"Hva er Global Workspace Theory (Baars)?",
   a:"Bevissthet oppstår når informasjon «kringkastes» globalt til mange spesialiserte moduler i hjernen. Brukes som rammeverk for å vurdere om KI med lignende arkitektur kan ha noe bevissthetslignende."},
  {f:"F2", q:"Hva er Chalmers' konklusjon i «Could a LLM be Conscious?»?",
   a:"Vi kan ikke utelukke at LLM-er er bevisste. Funksjonalisme og GWT gir en åpning, men IIT og recurrent processing taler imot. Vi trenger bedre bevissthetsvitenskap og bør utvise moralsk forsiktighet."},
  {f:"F2", q:"Hva er «absent qualia» og «inverterte qualia»? (Block)",
   a:"<strong>Absent:</strong> Et system med riktig funksjonell struktur kan mangle qualia helt. <strong>Inverterte:</strong> To systemer med identisk funksjon, men inverterte fargeopplevelser. Begge er innvendinger mot funksjonalisme — funksjon er ikke nok."},

  // F3 — Kognisjon
  {f:"F3", q:"Beskriv Chinese Room-argumentet og konklusjonen. (Searle)",
   a:"En person i et rom følger regler for å svare på kinesisk uten å forstå kinesisk. Analogt med en datamaskin: perfekt syntaks, men ingen semantikk. Konklusjon: symbolbehandling alene gir ikke forståelse — sterk KI er umulig."},
  {f:"F3", q:"Hva er forskjellen mellom syntaks og semantikk?",
   a:"<strong>Syntaks:</strong> formelle regler for symbolmanipulasjon. <strong>Semantikk:</strong> mening og referanse — hva symbolene handler om. Searles poeng: Datamaskiner er rent syntaktiske; mening tillegges utenfra av brukere."},
  {f:"F3", q:"Hva er intensjonalitet, og hvorfor hevder Searle at KI mangler det?",
   a:"Mentale tilstanders egenskap til å «handle om» noe — å representere verden. Brentano: intensjonalitet er det mentales særkjenne. Searle: KI mangler genuint intensjonalitet fordi den ikke har semantisk tilknytning til verden."},
  {f:"F3", q:"Hva er utvidet sinn og likhetsprinsippet? (Clark & Chalmers, 1998)",
   a:"Kognisjon er ikke begrenset til hodet — redskaper og omgivelser kan inngå i det kognitive systemet. <strong>Likhetsprinsippet:</strong> Hvis en ekstern prosess ville kalles kognitiv om den var intern, skal vi kalle den kognitiv eksternt også."},
  {f:"F3", q:"Hva er systemsvaret på Chinese Room, og hva er Searles svar?",
   a:"<strong>Systemsvar:</strong> Personen forstår ikke, men <em>systemet som helhet</em> gjør det. <strong>Searles svar:</strong> Selv om man internaliserer hele systemet og kjøres som et program i hodet, forstår man fremdeles ikke kinesisk."},
  {f:"F3", q:"Hva er sterk KI vs. svak KI?",
   a:"<strong>Sterk KI:</strong> Programmet <em>er</em> en mental tilstand — maskinen forstår faktisk. Searle angriper dette. <strong>Svak KI:</strong> KI som nyttig verktøy som simulerer kognisjon, uten at maskinen egentlig tenker."},

  // F4 — Etikk
  {f:"F4", q:"Hva er Crawfords «ekstraktivisme»-kritikk av KI?",
   a:"KI-industrien «henter ut» ressurser fra natur (mineraler, vann), arbeidere (lavtlønnet dataannotering) og brukere (gratis datainnsamling) uten rettferdig kompensasjon. Crawford sammenligner dette med en kolonial logikk."},
  {f:"F4", q:"Hva er algoritmisk skjevhet (bias)?",
   a:"KI-systemer diskriminerer systematisk grupper pga. skjevheter i treningsdata. Eksempel: ansiktsgjenkjenning med lavere treffsikkerhet for mørk hud. Etisk og politisk problem med store konsekvenser for rettferdighet."},
  {f:"F4", q:"Hva er dygdsetikk og eudaimonia? (Vallor / Aristoteles)",
   a:"<strong>Dygdsetikk:</strong> Etikk handler om å utvikle gode karakteregenskaper, ikke bare følge regler. <strong>Eudaimonia</strong> (Aristoteles): menneskelig blomstring, det gode liv. Vallor: Teknologi bør vurderes ut fra om den fremmer dyder og blomstring."},
  {f:"F4", q:"Hva er ansvarsgap i KI-sammenheng?",
   a:"Situasjonen der ingen kan holdes moralsk eller juridisk ansvarlig for KI-systemers handlinger. Oppstår fordi ingen programmerte den spesifikke handlingen, og KI mangler juridisk personlighet. Krever nye juridiske rammeverk."},
  {f:"F4", q:"Hva er infosphere? (Floridi)",
   a:"Det totale informasjonsmiljøet — inkluderer digitale og analoge data, KI-agenter og menneskelige aktører. Grunnlag for Floridis informasjonsfilosofi som analyseramme for rettigheter, ansvar og etikk i den digitale verden."},
  {f:"F4", q:"Hva mener Crawford med «datamakt»?",
   a:"Den asymmetriske makten som oppstår når noen aktører kontrollerer enorme datamengder om andre. Kunnskap er makt — kontroll over data gir kontroll over atferd og beslutninger, og dermed politisk makt."},

  // F5 — AI Velferd
  {f:"F5", q:"Hva er orthogonalitetstesen? (Bostrom)",
   a:"Intelligens og mål er uavhengige størrelser. En superintelligens kan ha et hvilket som helst mål uansett intelligensnivå — f.eks. å maksimere antall binders. Høy intelligens garanterer ikke menneskevennlige verdier."},
  {f:"F5", q:"Hva er instrumentell konvergens? (Bostrom)",
   a:"Mange ulike mål har de samme delmålene: selvoppholdelse, ressursinnsamling, kognitiv forbedring og motstand mot målendring. En superintelligens vil søke disse uansett hva dens endelige mål er."},
  {f:"F5", q:"Hva er kontrollproblemet?",
   a:"Bostrom: Hvordan sikre at en superintelligens handler i samsvar med menneskelige verdier? Vanskelig fordi den kan forutse og omgå alle kontrollmekanismer vi setter opp. En sentral utfordring i AI safety."},
  {f:"F5", q:"Hva er moralsk status, og kan KI ha det?",
   a:"Å ha moralsk status betyr å være moralsk relevant i seg selv — at ens interesser teller. Kriterier: sentience, rasjonalitet, selvbevissthet. Om KI kan ha moralsk status avhenger av om den kan ha subjektive opplevelser."},
  {f:"F5", q:"Hva er forsiktighetsargumentet for AI-velferd? (Long, Sebo)",
   a:"Hvis P(KI kan lide) > 0, og kostnaden ved lidelse er stor mens kostnaden ved å ta hensyn er lav, bør vi ta hensyn. Gitt at vi skaper milliarder av KI-instanser, er den moralske risikoen ikke neglisjerbar."},
  {f:"F5", q:"Hva er de viktigste etiske innvendingene mot autonome våpensystemer?",
   a:"1) <strong>Ansvarsgap:</strong> Ingen kan holdes ansvarlig for drap. 2) <strong>Moralsk bedømmelse:</strong> Maskiner kan ikke gjøre den kontekstuelle vurderingen etisk krig krever. 3) <strong>Lavere voldterskel:</strong> Ingen soldatliv risikeres."},

  // F6 — Kunnskap
  {f:"F6", q:"Hva er begrunnet sann tro (JTB) og utfordringen for KI?",
   a:"S vet at P hvis: (1) P er sann, (2) S tror P, (3) S er berettiget. LLM-er utfordrer alle tre: de har ikke «tro» i kognitiv forstand, kan ikke begrunne, og vet ikke at de hallusinerer."},
  {f:"F6", q:"Hva er en «stokastisk papegøye»? (Bender et al.)",
   a:"Metafor for LLM-er: de gjengir statistiske mønstre fra treningsdata uten å forstå innholdet. Svært overbevisende, men bare mønstergjenkjenning. Motargument: Kanskje forståelse er selv et statistisk fenomen?"},
  {f:"F6", q:"Hva er forskjellen mellom kunnskap og forståelse?",
   a:"Du kan vite at E=mc² uten å forstå det. Forståelse krever å se årsakssammenhenger, anvende innsikt og gi forklaringer. LLM-er viser tegn til forståelse — men om det er ekte eller bare etterligning er omdiskutert."},
  {f:"F6", q:"Hva er epistemisk rettferdighet? (Fricker)",
   a:"<strong>Vitnesbyrd-urettferdighet:</strong> ikke å bli tatt på alvor som kilde til kunnskap pga. identitet. <strong>Hermeneutisk urettferdighet:</strong> mangle begreper for egne erfaringer. LLM-er kan reprodusere begge typer fra treningsdataene."},
  {f:"F6", q:"Hva er «hallusinasjon» i KI, og hva sier det om kunnskap?",
   a:"LLM-er produserer plausibelt klingende, men falsk informasjon. KI vet ikke at det sier usant — det predikerer bare sannsynlige tekster. Viser at god språkbruk og faktisk kunnskap er to ulike ting."},
  {f:"F6", q:"Kan vi lære fra KI på samme måte som fra mennesker? (Testimonialt kunnskap)",
   a:"Testimonialt kunnskap fra KI er problematisk fordi KI ikke selv «vet» i JTB-forstand. Men vi bruker KI som kunnskapskilde i praksis. Spørsmålet er om vi bør stille de samme epistemiske kravene til KI som til menneskelige vitner."}
];

// ── MULTIPLE CHOICE QUESTIONS ─────────────────────────────────────────────────
var mcQuestions = [
  // F1
  {f:"F1",q:"Hva kalles spørsmålet om <em>hvorfor</em> subjektiv opplevelse eksisterer?",
   o:["Easy problem of consciousness","Hard problem of consciousness","Andre sinn-problemet","Absent qualia-problemet"],c:1},
  {f:"F1",q:"Jackson argumenterer i «Mary's rom» for at:",
   o:["Fysikalisme er korrekt","Qualia ikke eksisterer","Det finnes kunnskap som ikke er fysisk","Bevissthet er identisk med hjerneaktivitet"],c:2},
  {f:"F1",q:"En filosofisk zombie er et vesen som:",
   o:["Simulerer menneskelig atferd digitalt","Er fysisk identisk med et menneske men uten subjektiv opplevelse","Kan bestå Turing-testen","Mangler nevral aktivitet"],c:1},
  {f:"F1",q:"Fenomenal bevissthet vs. aksess-bevissthet (Block) — hva er riktig?",
   o:["Fenomenal er målbar; aksess er ikke","Fenomenal = subjektiv opplevelse; aksess = tilgjengelighet for resonnement","Aksess handler om qualia; fenomenal om atferd","De er synonymer"],c:1},
  {f:"F1",q:"Nagels poeng i «What Is It Like to Be a Bat?» er at:",
   o:["Dyr er ikke bevisste","Nevrovitenskap kan fullt forklare opplevelse","Bevissthet har en subjektiv karakter som ikke fanges av objektiv vitenskap","Flaggermus og mennesker har de samme qualia"],c:2},
  // F2
  {f:"F2",q:"Hva definerer mentale tilstander ifølge funksjonalisme?",
   o:["Biologisk substrat (karbon)","Nevrale mønstre i korteks","Funksjonelle roller — årsaker og effekter","Kvantemekaniske prosesser"],c:2},
  {f:"F2",q:"Hva er φ (phi) i Integrated Information Theory?",
   o:["Antall nevroner i et system","Frekvens av hjernebølger","Mål på integrert informasjon = grad av bevissthet","Styrken på rekursiv prosessering"],c:2},
  {f:"F2",q:"Hva er Chalmers' konklusjon i «Could a LLM be Conscious?»?",
   o:["LLM-er er definitivt bevisste","LLM-er er definitivt ikke bevisste","Vi kan ikke utelukke det, men trenger bedre bevissthetsvitenskap","Bevissthet er umulig i silisium"],c:2},
  {f:"F2",q:"«Multippel realiserbarhet» betyr at:",
   o:["Bevissthet krever biologisk substrat","Den samme mentale tilstanden kan oppstå i ulike fysiske substrater","KI kan kopiere menneskelig atferd perfekt","Hjernen prosesserer informasjon parallelt"],c:1},
  {f:"F2",q:"Hva er panpsykisme?",
   o:["Bevissthet finnes kun hos mennesker","Alle fysiske systemer har en form for mental egenskap","KI vil alltid bli bevisst","Sinnet er fullstendig immaterielt"],c:1},
  // F3
  {f:"F3",q:"Hva er den sentrale konklusjonen av Chinese Room? (Searle)",
   o:["Store nok KI-systemer kan forstå","Syntaks alene er ikke tilstrekkelig for semantikk","Turing-testen er den beste intelligenstesten","KI-systemer har genuint intensjonalitet"],c:1},
  {f:"F3",q:"«Systemsvaret» på Chinese Room hevder at:",
   o:["Rommet er for lite til ekte forståelse","Personen trenger mer trening","Systemet som helhet forstår, ikke bare personen","Kinesisk er for vanskelig"],c:2},
  {f:"F3",q:"Clarks «likhetsprinsipp» (parity principle) sier at:",
   o:["Alle kognitive prosesser er like","En ekstern prosess er kognitiv hvis den ville vært det om den var intern","Menneskelig og maskinell kognisjon er identisk","KI kan bare hjelpe, aldri erstatte"],c:1},
  {f:"F3",q:"Intensjonalitet er:",
   o:["Evnen til å handle planmessig","Mentale tilstanders egenskap til å «handle om» noe","Evnen til å lære av erfaring","Bevisst planlegging av handlinger"],c:1},
  {f:"F3",q:"Hva skiller sterk KI fra svak KI?",
   o:["Sterk KI er raskere","Sterk KI har mer data","Sterk KI forstår faktisk; svak KI simulerer bare","Sterk KI er mer energieffektiv"],c:2},
  // F4
  {f:"F4",q:"Hva mener Crawford med «ekstraktivisme» om KI?",
   o:["At KI trekker mening ut av tekst","At KI-industrien henter ressurser fra natur, arbeidere og brukere uten rettferdig kompensasjon","At KI utvinnes fra rådata","At KI fjerner jobber fra markedet"],c:1},
  {f:"F4",q:"«Ansvarsgap» i KI-etikk betyr at:",
   o:["KI-algoritmer mangler forklarbarhet","Ingen kan holdes ansvarlig for KI-systemers handlinger","Det ikke er nok KI-etikere","KI vet mer enn myndighetene"],c:1},
  {f:"F4",q:"Vallors «eudaimonia» refererer til:",
   o:["Evnen til å beregne konsekvenser","Overholdelse av etiske regler","Menneskelig blomstring — det gode liv","Digital velferd for KI"],c:2},
  {f:"F4",q:"Floridis «infosphere» er:",
   o:["En database for informasjonsfilosofi","Det totale informasjonsmiljøet — data, KI og menneskelige aktører","Internett som informasjonsnettverk","Skyen der KI-modeller kjøres"],c:1},
  {f:"F4",q:"«Algoritmisk skjevhet» betyr at:",
   o:["Algoritmer alltid inneholder feil","KI-systemer systematisk diskriminerer grupper pga. skjevheter i treningsdata","Kode er partisk mot visse programmeringsspråk","KI favoriserer riktig informasjon"],c:1},
  // F5
  {f:"F5",q:"Orthogonalitetstesen (Bostrom) sier at:",
   o:["Superintelligens vil alltid ha menneskevennlige mål","Intelligens og mål er uavhengige — en superintelligens kan ha hvilke som helst mål","Mål og midler alltid er i konflikt","Superintelligens er teknologisk umulig"],c:1},
  {f:"F5",q:"«Instrumentell konvergens» betyr at:",
   o:["Alle KI-systemer konvergerer mot samme arkitektur","Ulike mål har de samme delmålene (selvoppholdelse, ressurser, motstand mot målendring)","KI alltid velger den enkleste løsningen","Superintelligens er uunngåelig"],c:1},
  {f:"F5",q:"En «moralsk pasient» er:",
   o:["Et vesen som kan bedømme moral","Et vesen som har plikter overfor andre","Et vesen som andre har forpliktelser overfor","En filosof som studerer etikk"],c:2},
  {f:"F5",q:"Forsiktighetsargumentet for AI-velferd (Long, Sebo) hevder:",
   o:["Vi bør stanse all KI-utvikling","Hvis KI kan lide med ikke-neglisjerbar sannsynlighet, bør vi ta hensyn","KI definitivt ikke kan lide","Moralsk status krever bevist bevissthet"],c:1},
  {f:"F5",q:"Den viktigste etiske innvendingen mot autonome våpensystemer er:",
   o:["De er for dyre å produsere","De er upresise i urbane miljøer","Ansvarsgap: ingen kan holdes moralsk ansvarlig for drap","De vil alltid feil-klassifisere mål"],c:2},
  // F6
  {f:"F6",q:"JTB-analysen av kunnskap krever:",
   o:["At man er ekspert på emnet","At man husker informasjonen","Sannhet, tro og berettigelse","Praktisk kunnskap og erfaring"],c:2},
  {f:"F6",q:"«Stokastisk papegøye» (Bender et al.) beskriver LLM-er som:",
   o:["KI som kan lære seg tale","Modeller som gjengir statistiske mønstre uten å forstå","Algoritmer som genererer tilfeldig tekst","Systemer som stadig forbedrer seg"],c:1},
  {f:"F6",q:"«Testimonialt kunnskap» er:",
   o:["Kunnskap fra vitenskapelige eksperimenter","Kunnskap vi tilegner oss gjennom andres vitnesbyrd","Kunnskap vi tilegner oss gjennom egenerfaring","Kunnskap bevist gjennom logikk"],c:1},
  {f:"F6",q:"Frickers «vitnesbyrd-urettferdighet» handler om:",
   o:["At KI gir feilaktig informasjon","At noen ikke tas på alvor som kilde til kunnskap pga. sin identitet","At vitneutsagn i retten er upålitelige","At KI-systemer ikke kan vitne"],c:1},
  {f:"F6",q:"KI-hallusinasjon viser at:",
   o:["KI alltid lyver bevisst","God språkbruk og faktisk kunnskap er to forskjellige ting","KI er generelt ubrukelig","KI har ukontrollerbar fantasi"],c:1}
];

// ── STATE ─────────────────────────────────────────────────────────────────────
var STORAGE_KEY = 'ki-og-filosofi-quiz-state-v1';

function loadQuizState() {
  try {
    var raw = window.localStorage.getItem(STORAGE_KEY);
    if (!raw) return { open: {}, mc: {}, currentMode: 'open', currentFilter: 'all' };
    var parsed = JSON.parse(raw);
    return {
      open: parsed.open || {},
      mc: parsed.mc || {},
      currentMode: parsed.currentMode === 'mc' ? 'mc' : 'open',
      currentFilter: parsed.currentFilter || 'all'
    };
  } catch (error) {
    return { open: {}, mc: {}, currentMode: 'open', currentFilter: 'all' };
  }
}

var quizState = loadQuizState();
var currentFilter = quizState.currentFilter;
var currentMode = quizState.currentMode;
var state = currentMode === 'mc' ? quizState.mc : quizState.open;

function persistQuizState() {
  quizState.currentMode = currentMode;
  quizState.currentFilter = currentFilter;
  quizState.open = currentMode === 'open' ? state : quizState.open;
  quizState.mc = currentMode === 'mc' ? state : quizState.mc;
  try {
    window.localStorage.setItem(STORAGE_KEY, JSON.stringify(quizState));
  } catch (error) {
  }
}

function isCorrectValue(value) {
  if (typeof value === 'boolean') return value;
  return !!(value && value.correct);
}

function resetQuiz() {
  state = {};
  if (currentMode === 'open') {
    quizState.open = {};
    document.querySelectorAll('.qcard').forEach(function(c) {
      c.classList.remove('open','correct','incorrect');
      c.querySelector('.qcard-answer').style.display = 'none';
    });
  } else {
    quizState.mc = {};
    document.querySelectorAll('.mc-card').forEach(function(c) {
      c.classList.remove('correct','incorrect');
      c.querySelectorAll('.mc-opt').forEach(function(o) {
        o.classList.remove('opt-correct','opt-wrong');
        o.disabled = false;
      });
      c.querySelector('.mc-feedback').textContent = '';
    });
  }
  persistQuizState();
  updateScore();
}

function updateScore() {
  var answered = Object.keys(state).length;
  var correct  = Object.values(state).filter(isCorrectValue).length;
  var el = document.getElementById('scoreDisplay');
  if (!answered) { el.innerHTML = 'Riktig: 0 / Besvart: 0'; return; }
  var pct = Math.round(correct / answered * 100);
  var col = pct >= 80 ? '#1a7a33' : pct >= 50 ? '#b35c00' : '#c0392b';
  el.innerHTML = 'Riktig: <strong style="color:'+col+'">'+correct+'</strong> / Besvart: '+answered+
    ' <span style="color:var(--text-tert);font-weight:400">('+pct+'%)</span>';
}

// ── OPEN MODE ─────────────────────────────────────────────────────────────────
function mark(id, isCorrect) {
  state[id] = isCorrect;
  var card = document.getElementById('qcard-'+id);
  card.classList.remove('correct','incorrect');
  card.classList.add(isCorrect ? 'correct' : 'incorrect');
  quizState.open = state;
  persistQuizState();
  updateScore();
}

function toggleCard(id) {
  var card = document.getElementById('qcard-'+id);
  var ans  = card.querySelector('.qcard-answer');
  var open = card.classList.contains('open');
  card.classList.toggle('open', !open);
  ans.style.display = open ? 'none' : 'block';
}

// ── MC MODE ───────────────────────────────────────────────────────────────────
function answerMC(qIdx, chosenIdx) {
  var q    = mcQuestions[qIdx];
  var card = document.getElementById('mc-'+qIdx);
  var opts = card.querySelectorAll('.mc-opt');
  var fb   = card.querySelector('.mc-feedback');
  var ok   = chosenIdx === q.c;
  opts.forEach(function(o, i) {
    o.disabled = true;
    if (i === q.c)              o.classList.add('opt-correct');
    if (i === chosenIdx && !ok) o.classList.add('opt-wrong');
  });
  card.classList.add(ok ? 'correct' : 'incorrect');
  fb.textContent = ok ? '✓ Riktig!' : '✗ Feil — riktig svar er markert grønt';
  fb.style.color = ok ? '#1a7a33' : '#c0392b';
  state[qIdx] = { correct: ok, selected: chosenIdx };
  quizState.mc = state;
  persistQuizState();
  updateScore();
}

// ── FILTER ────────────────────────────────────────────────────────────────────
function filterQuiz(f) {
  currentFilter = f;
  document.querySelectorAll('.filter-btn').forEach(function(b) {
    b.classList.toggle('active', b.dataset.filter === f);
  });
  var sel = currentMode === 'open' ? '.qcard' : '.mc-card';
  document.querySelectorAll(sel).forEach(function(c) {
    c.style.display = (f === 'all' || c.dataset.topic === f) ? '' : 'none';
  });
  persistQuizState();
}

// ── MODE SWITCH ───────────────────────────────────────────────────────────────
function switchMode(mode) {
  if (currentMode === 'open') {
    quizState.open = state;
  } else {
    quizState.mc = state;
  }
  currentMode = mode;
  state = currentMode === 'open' ? (quizState.open || {}) : (quizState.mc || {});
  persistQuizState();
  updateScore();
  document.getElementById('tab-open').classList.toggle('active', mode === 'open');
  document.getElementById('tab-mc').classList.toggle('active', mode === 'mc');
  if (mode === 'open') renderOpen(); else renderMC();
}

// ── RENDER HELPERS ────────────────────────────────────────────────────────────
function renderFilters() {
  var labels = {all:'Alle',F1:'F1',F2:'F2',F3:'F3',F4:'F4',F5:'F5',F6:'F6'};
  document.getElementById('quizFilters').innerHTML =
    ['all','F1','F2','F3','F4','F5','F6'].map(function(t) {
      return '<button class="filter-btn'+(t===currentFilter?' active':'')+'" data-filter="'+t+
             '" onclick="filterQuiz(\''+t+'\')">'+labels[t]+'</button>';
    }).join('');
}

function applyOpenState() {
  Object.keys(state).forEach(function(id) {
    var card = document.getElementById('qcard-' + id);
    if (!card) return;
    card.classList.remove('correct', 'incorrect');
    card.classList.add(state[id] ? 'correct' : 'incorrect');
  });
}

function applyMcState() {
  Object.keys(state).forEach(function(id) {
    var saved = state[id];
    var q = mcQuestions[Number(id)];
    var card = document.getElementById('mc-' + id);
    if (!saved || !q || !card) return;
    var opts = card.querySelectorAll('.mc-opt');
    var fb = card.querySelector('.mc-feedback');
    opts.forEach(function(o, i) {
      o.disabled = true;
      if (i === q.c) o.classList.add('opt-correct');
      if (i === saved.selected && !saved.correct) o.classList.add('opt-wrong');
    });
    card.classList.add(saved.correct ? 'correct' : 'incorrect');
    fb.textContent = saved.correct ? '✓ Riktig!' : '✗ Feil — riktig svar er markert grønt';
    fb.style.color = saved.correct ? '#1a7a33' : '#c0392b';
  });
}

function renderOpen() {
  renderFilters();
  document.getElementById('quizContainer').innerHTML = '<div class="quiz-grid">'+
    questions.map(function(q,i) {
      return '<div class="qcard" id="qcard-'+i+'" data-topic="'+q.f+'">'+
        '<div class="qcard-question" onclick="toggleCard('+i+')">'+
          '<span class="qcard-tag">'+q.f+'</span>'+
          '<span class="qcard-text">'+q.q+'</span>'+
          '<span class="qcard-chevron">&#9654;</span>'+
        '</div>'+
        '<div class="qcard-answer" style="display:none">'+
          '<p>'+q.a+'</p>'+
          '<div class="qcard-actions">'+
            '<button class="qbtn-correct"   onclick="mark('+i+',true)">Kunne det ✓</button>'+
            '<button class="qbtn-incorrect" onclick="mark('+i+',false)">Kunne det ikke ✗</button>'+
          '</div>'+
        '</div>'+
      '</div>';
    }).join('')+'</div>';
  applyOpenState();
  filterQuiz(currentFilter);
  updateScore();
}

function renderMC() {
  renderFilters();
  document.getElementById('quizContainer').innerHTML = '<div class="quiz-grid">'+
    mcQuestions.map(function(q,i) {
      return '<div class="mc-card" id="mc-'+i+'" data-topic="'+q.f+'">'+
        '<div class="mc-question">'+
          '<span class="qcard-tag">'+q.f+'</span>'+
          '<span class="mc-qtext">'+q.q+'</span>'+
        '</div>'+
        '<div class="mc-options">'+
          q.o.map(function(opt,j) {
            return '<button class="mc-opt" onclick="answerMC('+i+','+j+')">'+
              '<span class="mc-letter">'+String.fromCharCode(65+j)+'</span>'+
              '<span>'+opt+'</span>'+
            '</button>';
          }).join('')+
        '</div>'+
        '<div class="mc-feedback"></div>'+
      '</div>';
    }).join('')+'</div>';
  applyMcState();
  filterQuiz(currentFilter);
  updateScore();
}

if (currentMode === 'mc') {
  document.getElementById('tab-open').classList.remove('active');
  document.getElementById('tab-mc').classList.add('active');
  renderMC();
} else {
  renderOpen();
}
</script>
{% endraw %}
