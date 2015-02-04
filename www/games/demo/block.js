(function(){var imported=document.createElement("script");imported.src="http://robotmoose.com/games/demo/sprite.js";document.head.appendChild(imported);})();

function block_t(x,y)
{
	var myself=this;

	myself.x=x;
	myself.y=y;
	myself.spr=new sprite_t("http://robotmoose.com/games/demo/block.png",1);

	myself.draw=function(simulation)
	{
		if(simulation)
		{
			simulation.ctx.save();
			simulation.ctx.translate(myself.x,myself.y);
			myself.spr.draw(simulation);
			simulation.ctx.restore();
		}
	};
};