module.exports = function(eleventyConfig) {
  // Passthrough copy for PDFs
  eleventyConfig.addPassthroughCopy({ "content/pdfs": "pdfs" });
  // copy stylesheet directly from content
  eleventyConfig.addPassthroughCopy({ "content/styles.css": "styles.css" });

  return {
    dir: {
      input: "content",
      output: "site",
      includes: "_includes",
      data: "_data"
    },
    markdownTemplateEngine: "njk",
    htmlTemplateEngine: "njk",
    templateFormats: ["md", "njk", "html"]
  };
};
