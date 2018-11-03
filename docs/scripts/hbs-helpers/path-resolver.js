module.exports = function(path,options){
  return path.name === 'index'? path.dir:path.dir+'/'+path.name;
};
