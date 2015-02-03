(function(){var imported=document.createElement("script");imported.src="http://robotmoose.com/games/demo/block.js";document.head.appendChild(imported);})();

var level_t=function(places)
{
	this.blocks=new Array();

	for(var ii=0;ii<places.length;++ii)
		this.blocks[ii]=new block_t(places[ii].x,places[ii].y);

	this.draw=function(simulation)
	{
		for(var ii=0;ii<this.blocks.length;++ii)
			this.blocks[ii].draw(simulation);
	};
};