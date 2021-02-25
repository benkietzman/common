Setup:
  In your node scripts dir, build a symbolic link to the common
  node_modules dir 
  run "ln -s /scripts/common/nodejs/node_modules node_modules"

Usage:
  var common = require('common');
  var junction = new common.ServiceJunction();

NPM Instructions:
  Upgrading Node Modules:
    run "npm update"
  Installing a new Node Module:
    run "npm install <moduleName> --save"
