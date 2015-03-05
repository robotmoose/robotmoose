(function(){var imported=document.createElement("script");imported.src="/games/demo/sprite.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="/games/demo/level.js";document.head.appendChild(imported);})();

function moose_t(x,y)
{
	var myself=this;

	myself.x=x;
	myself.y=y;
	myself.spr_right=new sprite_t("/games/demo/moose_right.png",4);
	myself.spr_left=new sprite_t("/games/demo/moose_left.png",4);
	myself.spr_jump=new sprite_t("/games/demo/moose_jump.png",2);
	myself.spr=myself.spr_right;
	myself.speed=100;
	myself.animation_speed=20;
	myself.jump=false;
	myself.direction=1;
	myself.bb={width:0,height:0};
	myself.v_speed=0;
	myself.y_velocity=0;

	myself.loop=function(simulation,dt,level)
	{
		if(simulation)
		{
			//Check for Under Collision
			var collision=false;
			var new_y=myself.y+myself.y_velocity;
			for(var ii=0;ii<level.blocks.length;++ii)
			{
				if(myself.x+myself.bb.width/2.0>=level.blocks[ii].x-level.blocks[ii].spr.width/2.0&&
					myself.x-myself.bb.width/2.0<=level.blocks[ii].x+level.blocks[ii].spr.width/2.0&&
					new_y+myself.bb.height/2.0>=level.blocks[ii].y-level.blocks[ii].spr.height/2.0&&
					new_y-myself.bb.height/2.0<=level.blocks[ii].y+level.blocks[ii].spr.height/2.0)
				{
					collision=true;
					myself.y_velocity=0;
					myself.jump=false;
					break;
				}
			}
			if(!collision)
			{
				myself.y=new_y;
				myself.y_velocity+=9.8*dt;
			}
			if(myself.y_velocity>100)
				myself.y_velocity=100;
			if(myself.y_velocity<-100)
				myself.y_velocity=-100;

			//Move Left/Right
			var moved=false;
			if(simulation.keys_down[kb_right]&&!simulation.keys_down[kb_left])
			{
				moved=true;
				myself.direction=1;
			}
			if(!simulation.keys_down[kb_right]&&simulation.keys_down[kb_left])
			{
				moved=true;
				myself.direction=-1;
			}
			if(moved)
			{
				var collision=false;
				var new_x=myself.x+myself.speed*dt*myself.direction;

				for(var ii=0;ii<level.blocks.length;++ii)
				{
					if(new_x+myself.bb.width/2.0>=level.blocks[ii].x-level.blocks[ii].spr.width/2.0&&
						new_x-myself.bb.width/2.0<=level.blocks[ii].x+level.blocks[ii].spr.width/2.0&&
						myself.y+myself.bb.height/2.0>=level.blocks[ii].y-level.blocks[ii].spr.height/2.0&&
						myself.y-myself.bb.height/2.0<=level.blocks[ii].y+level.blocks[ii].spr.height/2.0)
					{
						collision=true;
						break;
					}
				}

				if(!collision)
					myself.x=new_x;
			}

			//Jump
			if(simulation.keys_pressed[kb_up]&&!myself.jump)
			{
				myself.jump=true;
				myself.y_velocity=-5;
			}
			if(myself.jump)
			{
				myself.spr=myself.spr_jump;

				if(myself.direction==1)
					myself.spr.frame=0;
				else
					myself.spr.frame=1;
			}
			else
			{
				if(myself.direction==1)
					myself.spr=myself.spr_right;
				else
					myself.spr=myself.spr_left;

				if(moved)
					myself.spr.frame+=myself.animation_speed*dt;
				else
					myself.spr.frame=0;
			}
		}
	};

	myself.draw=function(simulation)
	{
		if(simulation)
		{
			myself.bb.width=22;
			myself.bb.height=Math.max(myself.spr_right.height,myself.spr_left.height,myself.spr_jump.height);

			simulation.ctx.save();
			simulation.ctx.translate(myself.x,myself.y);
			myself.spr.draw(simulation);
			simulation.ctx.restore();
		}
	};
};