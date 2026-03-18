const fs = require('fs');
const path = require('path');

module.exports = () => {
  const dir = path.join(__dirname, '..', 'pdfs');
  let files = [];
  try {
    files = fs.readdirSync(dir).filter(f => f.endsWith('.pdf'));
  } catch (e) {
    // ignore
  }
  return files.map(f => ({ name: f, url: `/pdfs/${encodeURIComponent(f)}` }));
};
