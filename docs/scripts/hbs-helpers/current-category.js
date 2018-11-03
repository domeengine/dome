module.exports = function(test,options){
  if(this.path.dir === test)
  {
    return options.fn(this);
  }
  else {
    return options.inverse(this);
  }
};
