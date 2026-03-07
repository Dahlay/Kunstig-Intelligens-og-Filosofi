---
layout: layout.njk
title: PDF Resources
permalink: /pdfs.html
---

<h2>PDF Resources</h2>
<ul>
{% for pdf in pdfs %}
  <li><a href="{{ pdf.url }}">{{ pdf.name }}</a></li>
{% endfor %}
</ul>
