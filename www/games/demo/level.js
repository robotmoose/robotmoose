(function(){var imported=document.createElement("script");imported.src="http://robotmoose.com/games/demo/block.js";document.head.appendChild(imported);})();

function level_t(places)
{
	var myself=this;

	myself.blocks=new Array();

	for(var ii=0;ii<places.length;++ii)
		myself.blocks[ii]=new block_t(places[ii].x,places[ii].y);

	myself.draw=function(simulation)
	{
		for(var ii=0;ii<myself.blocks.length;++ii)
			myself.blocks[ii].draw(simulation);
	};
};