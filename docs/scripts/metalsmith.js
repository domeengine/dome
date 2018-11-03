var Metalsmith = require('metalsmith');
var markdown = require('metalsmith-markdown');
var layouts = require('metalsmith-layouts');
var assets = require('metalsmith-static');
var partials = require('metalsmith-discover-partials');
var collections = require('metalsmith-collections');
var helpers = require('metalsmith-register-helpers');
var paths = require('metalsmith-paths');
var permalinks = require('metalsmith-permalinks');

var ms = Metalsmith(__dirname)
  .source('../src/markdown')
  .destination('../build')
  .use(paths())
  .use(helpers({
    directory: './hbs-helpers'
  }))
  .use(collections({
    home: {
      pattern: 'index.md',
      metadata: {
        name: "Home"
      }
    },
    guide: {
      pattern: 'guides/**/*.md',
      metadata: {
        name: "Guide"
      }
    },
    api: {
      pattern: 'modules/**/*.md',
      metadata: {
        name: "API"
      }
    }
  }))
  .use(markdown())
  .use(layouts({
    engine: 'handlebars',
    directory: '../templates',
    default: 'template.hbs'
  }))
  .use(assets({
    src: '../src/assets',
    dest: '../build/assets'
  }))
  .use(assets({
    src: '../src/css',
    dest: '../build/assets/css'
  }))
  .use(permalinks({
    relative: false
  }))

exports.ms = ms;
