(function(){var imported=document.createElement("script");imported.src="http://robotmoose.com/games/demo/sprite.js";document.head.appendChild(imported);})();
(function(){var imported=document.createElement("script");imported.src="http://robotmoose.com/games/demo/level.js";document.head.appendChild(imported);})();

var moose_t=function(x,y)
{
	this.x=x;
	this.y=y;
	this.spr_right=new sprite_t("http://robotmoose.com/games/demo/moose_right.png",4);
	this.spr_left=new sprite_t("http://robotmoose.com/games/demo/moose_left.png",4);
	this.spr_jump=new sprite_t("http://robotmoose.com/games/demo/moose_jump.png",2);
	this.spr=this.spr_right;
	this.speed=100;
	this.animation_speed=20;
	this.jump=false;
	this.direction=1;
	this.bb={width:0,height:0};
	this.v_speed=0;

	this.y_velocity=0;

	this.loop=function(simulation,dt,level)
	{
		if(simulation)
		{
			//Check for Under Collision
			var collision=false;
			var new_y=this.y+this.y_velocity;
			for(var ii=0;ii<level.blocks.length;++ii)
			{
				if(this.x+this.bb.width/2.0>=level.blocks[ii].x-level.blocks[ii].spr.width/2.0&&
					this.x-this.bb.width/2.0<=level.blocks[ii].x+level.blocks[ii].spr.width/2.0&&
					new_y+this.bb.height/2.0>=level.blocks[ii].y-level.blocks[ii].spr.height/2.0&&
					new_y-this.bb.height/2.0<=level.blocks[ii].y+level.blocks[ii].spr.height/2.0)
				{
					collision=true;
					this.y_velocity=0;
					this.jump=false;
					break;
				}
			}
			if(!collision)
			{
				this.y=new_y;
				this.y_velocity+=9.8*dt;
			}
			if(this.y_velocity>100)
				this.y_velocity=100;
			if(this.y_velocity<-100)
				this.y_velocity=-100;

			//Move Left/Right
			var moved=false;
			if(simulation.keys_down[kb_right]&&!simulation.keys_down[kb_left])
			{
				moved=true;
				this.direction=1;
			}
			if(!simulation.keys_down[kb_right]&&simulation.keys_down[kb_left])
			{
				moved=true;
				this.direction=-1;
			}
			if(moved)
			{
				var collision=false;
				var new_x=this.x+this.speed*dt*this.direction;

				for(var ii=0;ii<level.blocks.length;++ii)
				{
					if(new_x+this.bb.width/2.0>=level.blocks[ii].x-level.blocks[ii].spr.width/2.0&&
						new_x-this.bb.width/2.0<=level.blocks[ii].x+level.blocks[ii].spr.width/2.0&&
						this.y+this.bb.height/2.0>=level.blocks[ii].y-level.blocks[ii].spr.height/2.0&&
						this.y-this.bb.height/2.0<=level.blocks[ii].y+level.blocks[ii].spr.height/2.0)
					{
						collision=true;
						break;
					}
				}

				if(!collision)
					this.x=new_x;
			}

			//Jump
			if(simulation.keys_pressed[kb_up]&&!this.jump)
			{
				this.jump=true;
				this.y_velocity-=5;
			}
			if(this.jump)
			{
				this.spr=this.spr_jump;

				if(this.direction==1)
					this.spr.frame=0;
				else
					this.spr.frame=1;
			}
			else
			{
				if(this.direction==1)
					this.spr=this.spr_right;
				else
					this.spr=this.spr_left;

				if(moved)
					this.spr.frame+=this.animation_speed*dt;
				else
					this.spr.frame=0;
			}
		}
	};

	this.draw=function(simulation)
	{
		if(simulation)
		{
			this.bb.width=22;
			this.bb.height=Math.max(this.spr_right.height,this.spr_left.height,this.spr_jump.height);

			simulation.ctx.save();
			simulation.ctx.translate(this.x,this.y);
			this.spr.draw(simulation);
			simulation.ctx.restore();
		}
	};
};