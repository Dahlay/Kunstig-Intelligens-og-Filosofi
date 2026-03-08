---
layout: layout.njk
title: "Quiz"
permalink: /quiz/index.html
---

<div class="lecture-header">
  <div class="lecture-number">Selvtest</div>
  <h1>Quiz</h1>
  <p class="lecturer">Klikk på et spørsmål for å se svaret — merk deg selv</p>
  <div class="lecture-meta">
    <span class="lecture-tag">36 spørsmål</span>
    <span class="lecture-tag">F1–F6</span>
    <span class="lecture-tag">Selvvurdering</span>
  </div>
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
var questions = [
  // F1 — Bevissthet I
  {f:"F1", q:"Hva er «hard problem of consciousness»?",
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

var state = {};
var currentFilter = 'all';

function resetQuiz() {
  state = {};
  document.querySelectorAll('.qcard').forEach(function(c) {
    c.classList.remove('open', 'correct', 'incorrect');
    c.querySelector('.qcard-answer').style.display = 'none';
  });
  updateScore();
}

function updateScore() {
  var answered = Object.keys(state).length;
  var correct = Object.values(state).filter(function(v) { return v; }).length;
  var el = document.getElementById('scoreDisplay');
  if (answered === 0) {
    el.innerHTML = 'Riktig: 0 / Besvart: 0';
    el.style.color = '';
  } else {
    var pct = Math.round(correct / answered * 100);
    var color = pct >= 80 ? '#1a7a33' : pct >= 50 ? '#b35c00' : '#c0392b';
    el.innerHTML = 'Riktig: <strong style="color:' + color + '">' + correct + '</strong> / Besvart: ' + answered + ' <span style="color:var(--text-tert);font-weight:400">(' + pct + '%)</span>';
  }
}

function mark(id, isCorrect) {
  state[id] = isCorrect;
  var card = document.getElementById('qcard-' + id);
  card.classList.remove('correct', 'incorrect');
  card.classList.add(isCorrect ? 'correct' : 'incorrect');
  updateScore();
}

function toggleCard(id) {
  var card = document.getElementById('qcard-' + id);
  var ans = card.querySelector('.qcard-answer');
  var isOpen = card.classList.contains('open');
  card.classList.toggle('open', !isOpen);
  ans.style.display = isOpen ? 'none' : 'block';
}

function filterQuiz(f) {
  currentFilter = f;
  document.querySelectorAll('.filter-btn').forEach(function(b) {
    b.classList.toggle('active', b.dataset.filter === f);
  });
  document.querySelectorAll('.qcard').forEach(function(c) {
    var match = f === 'all' || c.dataset.topic === f;
    c.style.display = match ? '' : 'none';
  });
}

function renderQuiz() {
  var labels = { all:'Alle', F1:'F1', F2:'F2', F3:'F3', F4:'F4', F5:'F5', F6:'F6' };
  var topics = ['all','F1','F2','F3','F4','F5','F6'];
  document.getElementById('quizFilters').innerHTML = topics.map(function(t) {
    return '<button class="filter-btn' + (t === 'all' ? ' active' : '') + '" data-filter="' + t + '" onclick="filterQuiz(\'' + t + '\')">' + labels[t] + '</button>';
  }).join('');

  document.getElementById('quizContainer').innerHTML = '<div class="quiz-grid">' +
    questions.map(function(q, i) {
      return '<div class="qcard" id="qcard-' + i + '" data-topic="' + q.f + '">' +
        '<div class="qcard-question" onclick="toggleCard(' + i + ')">' +
          '<span class="qcard-tag">' + q.f + '</span>' +
          '<span class="qcard-text">' + q.q + '</span>' +
          '<span class="qcard-chevron">&#9654;</span>' +
        '</div>' +
        '<div class="qcard-answer" style="display:none">' +
          '<p>' + q.a + '</p>' +
          '<div class="qcard-actions">' +
            '<button class="qbtn-correct" onclick="mark(' + i + ',true)">Kunne det ✓</button>' +
            '<button class="qbtn-incorrect" onclick="mark(' + i + ',false)">Kunne det ikke ✗</button>' +
          '</div>' +
        '</div>' +
      '</div>';
    }).join('') + '</div>';
}

renderQuiz();
</script>
{% endraw %}
