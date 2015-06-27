
Katies katies;
Button rstBtn;
Button goBtn;
boolean global_drag_lock = false;
boolean global_add_heading = false;
boolean global_waypoint_add = false;

Vector[] location = {new Vector(200,200), new Vector(300,300), new Vector(400,400)};

void setup() {
  size(720, 600);
  
  
  // initialize three Katie objects...
  katies = new Katies();
  katies.addKatie(108, 305, 1);
  katies.addKatie(580, 390, 2);
  katies.addKatie(330, 140, 3);
  
  //Create Buttons
  rstBtn = new Button(10, 525, 0);
  goBtn = new Button(10, 560, 1);
  
}

void draw(){
  background(color(170,220,250));
  fill(color(160,210,130));
  strokeWeight(6);
  line(150, 480, 150, 250);
  line(150, 190, 150, 100);
  line(150,100,550,100);
  line(550,100,550,190);
  line(550, 330, 550, 480);
  line(550,480,370,480);
  line(290,480,150,480);
  strokeWeight(1);
  katies.draw();
//  katies.keyKaties();
//  if (global_drag_lock) { katies.update(); }

  //Draw Button Interface
  rstBtn.drawButton(true);
  goBtn.drawButton(true); 
}

//Vector mouse = new Vector();
Vector offset = new Vector();


//Function to check if mouse button was clicked over drawn buttons
void mouseClicked() {
  if(mouseButton == LEFT) {
    if(rstBtn.clickButton() == true) {
      //reset katie to original position
      //currently happens only the first time...
      Katie katie;
      for (int i=0; i<katies.size(); i++) {
        katie = (Katie) katies.katies.get(i);
        //katie.resetKatie(location[i]);
        //println("Katie " + i + " reset.");
        katie.resetKatie();
      }
//      sendResetCommand(1);
    }
    if(goBtn.clickButton() == true) {
      //move katie to waypoint
      Katie katie;
      for (int i=0; i<katies.size(); i++) {
        katie = (Katie) katies.katies.get(i);
        //location[i].printVector();
        katie.calculateMovements();
        if (katie.waypoint1.a == 1) katie.move();
      }
    }
    rstBtn.clickButton();
    goBtn.clickButton();
  } 
}

void mouseReleased() {
  katies.resetLocks();
  global_drag_lock = false;
}

void mousePressed() {
  if ( global_add_heading == true ) {
    Katie katie;
    for (int i=0; i<katies.size(); i++) {
     katie = (Katie) katies.katies.get(i);
      if (katie.change_heading == true) { 
        katie.setHeading(mouseX,mouseY);
        katie.change_heading = false;
      }   
    }
    global_add_heading = false;  
  }
  if (global_waypoint_add == true) {
    Katie katie;
    for (int i=0; i<katies.size(); i++) {
     katie = (Katie) katies.katies.get(i);
      if (katie.waypoint_add == true) { 
        katie.addWaypoint(mouseX,mouseY);
        katie.waypoint_add = false;
      }   
    }
  }
  global_waypoint_add = false;  
}
class Button {
  
  //Button parameters
  int x, y, id;
  String[] display_text = {"RESET", "GO"};
  int[] button_width = {60, 35};
  int button_height = 25;
  
  //Not currently used
  boolean active = true;
  
  public Button(int _x, int _y, int _id) {
    x = _x;
    y = _y;
    id = _id; 
  }
  
  void drawButton(boolean _active) {
    textSize(15);
    if(_active == true) {
      fill(153);
      stroke(204);
      rect(x, y, button_width[id], button_height);
      stroke(33);
      //bottom
      line(x, y + button_height, x + button_width[id], y + button_height);
      //right
      line(x  + button_width[id], y, x + button_width[id], y + button_height);
      fill(204);
      text(display_text[id], x + 5, y + button_height - 5);
    } else {
      fill(102);
      stroke(51);
      rect(x, y, button_width[id], button_height);
      stroke(222);
      //bottom
      line(x, y + button_height, x + button_width[id], y + button_height);
      //right
      line(x  + button_width[id], y, x + button_width[id], y + button_height);
      fill(51);
      text(display_text[id], x + 5, y + button_height - 5);
    } 
  }
  
  boolean onButton() {
    if (mouseX >= x && mouseX <= x+button_width[id] && 
      mouseY >= y && mouseY <= y+button_height) {
      return true;
    } else {
      return false;
    }
  } 
  
  boolean clickButton() {
    if(onButton()) {
      drawButton(false);
      /////Call Functions here
      //println(display_text[id]);
      return true;
    } 
//    else {
//      drawButton(true);
//    }
    return false;
  }
}
// Each object represents current place and plan to move using waypoints
public class Katie {
  Vector default_location = new Vector();
  Vector location = new Vector();
  Vector heading = new Vector();
  Vector influence = new Vector();
  Vector waypoint1 = new Vector();
  //Vector waypoint2 = new Vector();
  
  // data that will will send to Katie
  float angle1, distance1;
  //int angle2, distance2;
  

  int id;
  int radius = 20;
  boolean mouseOver = false;
  boolean drag_locked = false;
  boolean change_heading = false;
  boolean waypoint_add = false;
  String name;
  
  int debounceTime = 250;
  int lastKeyPress = 0;
  
  color[] cid = {color(204, 204, 204), color(0, 153, 204), color(204, 153, 0)};

  public Katie(float x, float y, int t_id) {
    location = new Vector(x, y);
    default_location = new Vector(x,y);
    waypoint1 = new Vector(x,y);
    //heading = new Vector(x + 20, y + 1); // default direction
    //atan2(y +  - location.y, x - location.x);
    setHeading(x+20, y+1);
    id = t_id;
  }
  
  public Katie(float x, float y, float vx, float vy, int t_id) {
    location = new Vector(x, y);
    heading = new Vector(vx, vy);
    id = t_id;
  }
  
  public void resetKatie() {
    location.set(default_location.x, default_location.y);
    setHeading(location.x + 20, location.y+1);
    waypoint1.a = 0;
  }
  
  void resetKatie(Vector _location) {
    location = _location;
    setHeading(_location.x + 20, _location.y + 1);
    display();
  }
  
  public void move() {
    setHeading(waypoint1.x, waypoint1.y);
    heading.set(heading.x + (waypoint1.x - location.x), heading.y + (waypoint1.y - location.y));
    location.set(waypoint1.x, waypoint1.y);
    waypoint1.a = 0;
  }
  
  class WayPoint {
    Vector location = new Vector();//float x, y, v;
    Vector velocity = new Vector();
    int queue_position;
    int behavior_type; // 0 is no type, 1 is call to others, 2 is scan, 3 is call and scan 
  }
  
  void addWaypoint(float x, float y) {
    waypoint1 = new Vector(x,y);
    waypoint1.a = 1; // 1 means active, 0 is off
//    waypoint1.x = x;
//    waypoint1.y = y;
    println(waypoint1.x + " " + waypoint1.y);
  }
  
  void resetLocks() {
    //mouseOver = false;
    //drag_locked = false;
    change_heading = false;
  }
  
  void display() {
    overKatie();
    clickKatie();
    keyInputOverKatie();
    
    if (drag_locked) {
      offset.set(location.x - mouseX, location.y - mouseY);
      location.set(location.x - offset.x, location.y - offset.y);
      heading.set(heading.x - offset.x, heading.y - offset.y);
    }
    
     // draw any guide lines first so they are under objects
    if (waypoint1.a == 1) line(location.x, location.y,waypoint1.x, waypoint1.y);

    
    fill(cid[id - 1]);
    ellipse(location.x,location.y,radius,radius);
    fill(255);
    ellipse(heading.x,heading.y,radius/3,radius/3);
    fill(cid[id - 1]);
    if (waypoint1.a == 1)
      ellipse(waypoint1.x,waypoint1.y,radius/2,radius/2);
    
  }
  
  
  void calculateMovements () {
    int angle = 0, distance = 0;
    
   if (waypoint1.a == 1) { 
     //// returns relative angle in degrees to the nearest 1 degree
     ////   angle between lines location-heading and location-waypoint1
      angle1 = degrees(atan2(waypoint1.y - location.y, waypoint1.x - location.x) 
        - atan2(heading.y - location.y, heading.x - location.x));
      angle = (int) round(angle1);
   
     //// returns distance in mm where the our radius value is equal to radius of the Create 
     ////   distance between points location and waypoint1
      distance1 =  getDistance(location,waypoint1);
      distance = (int) round(distance1);
      
      stackCommand(angle, distance, 200);
      sendCommandPacketWithId(id);
      println(id + " angle: " + angle + " distance: " + distance);
        
   } 
  }
  
  //Temporary to check clockwise and counterclockwise angles
  int calculateAngle(boolean clockwise) {
    int angle = 0;
    if (waypoint1.a == 1) {
       angle1 = degrees(atan2(waypoint1.y - location.y, waypoint1.x - location.x) 
          - atan2(heading.y - location.y, heading.x - location.x));
        angle = (int) round(angle1);
       if(clockwise == true) {
         return 360 + angle;
       }
       return angle;
    }
    return 0;
   }
  
  
  void setHeading(float x, float y) {
    float angle = atan2(y - location.y, x - location.x); // find the angle between new vector and current location
//    println(angle);
    heading.x = cos(angle)*(radius/2) + location.x;
    heading.y = sin(angle)*(radius/2) + location.y;
  }
  
  // Need to make so only one Katie can be selected at a time
  //   or all objects cannot be near each other, they push each 
  //   other out of the way.
  void overKatie() {
    if ( sqrt(sq(location.x - mouseX) + sq(location.y - mouseY)) < radius/2 ) {
      stroke(255); 
      fill(153);
      mouseOver = true;
    } else {
      stroke(153);
      fill(153);
      mouseOver = false;
    }
  }
   
  void clickKatie () {  
    if (mousePressed && (mouseButton == LEFT) && mouseOver) {
      drag_locked = true; // might not be needed.  
      global_drag_lock = true;
    } else {
      drag_locked = false;
    }
  }
    
  void keyInputOverKatie() {
//    int time = millis();
    if((keyPressed == true) && (mouseOver == true) && (lastKeyPress + debounceTime <= millis())) {
//      Check Statements
      println("Key = " + key + " and Katie Id = " + id + " and time = " + millis());
//      println(lastKeyPress + debounceTime);
        /** Keycode
          q,Q: set waypoint 1
          w,W: move to next waypoint
          e,E; set waypoint 2 (phase 2)
          a,A: turn counterclockwise
          s,S: change heading
          d,D: turn clockwise
          z,Z: beep
          x,X: scan
          c,C: other
        **/
        switch(key) {
          case 'q':
          case 'Q':
            //Set waypoint
            waypoint_add = true;
            global_waypoint_add = true;
            break;
          case 'w':
          case 'W':
            //Move to waypoint
            calculateMovements();
            move();            
            break;
          case 'e':
          case 'E':
            //Set Waypoint 2
            break;
          case 'a':
          case 'A':
            //Turn counterclockwise
            println("Angle of turn = " + calculateAngle(false));
            break;
          case 's':
          case 'S':
            change_heading = true;
            global_add_heading = true;
            break;
          case 'd':
          case 'D':
            //turn clockwise
            println("Angle of turn = " + calculateAngle(true));
            break;
          case 'z':
          case 'Z':
            break;
          case 'x':
          case 'X':
              //Turn counterclockwise
            break;
          case 'c':
          case 'C':
            break;
          default:
            //Unexpected keyboard command
            println("They key you have pressed does not have a function");
        }
      //Record last key press
      lastKeyPress = millis();
    }
  }

  void calcKatieKatieInfluences(Katie katiej){
    float d = getDistance(location, katiej.location);
    
    if (d < radius) {
      /// coupled to mouse  ///need mouse offset, which will help with heading issue too
      katiej.location.x = katiej.location.x - offset.x;
      katiej.location.y = katiej.location.y - offset.y;
      katiej.heading.x = katiej.heading.x - offset.x;
      katiej.heading.y = katiej.heading.y - offset.y;
    }
  }
}
public class Katies {
  ArrayList katies;
  
  public Katies() {
    katies = new ArrayList();
  }
  
  int size() {
    return katies.size();
  }
  
  void addKatie(float x, float y, int id) {
    katies.add(new Katie(x,y,id));
  }
  
//  void addKatie(int id) {
//    katies.add(new Katie(id));
//  }
  
  void draw() {
    Katie katie;
    for (int i=0; i<katies.size(); i++) {
      katie = (Katie) katies.get(i);
      katie.display();    
    }     
  }
  
  
//  void keyKaties () {  
//    Katie katie;
//    Katie katiej;
//    for (int i=0; i<katies.size(); i++) {
//      katie = (Katie) katies.get(i);
//      if (keyPressed && katie.mouseOver) {
//        switch(key) {
//          case 'h':
//          case 'H':
//          println("Change heading for Katie "+ katie.id);
//          katie.change_heading = true; 
//          global_add_heading = true;
//          //turn off change_heading of others...
//          for (int j=0; j<katies.size(); j++) {
//            if ( i == j ) continue;
//            katiej = (Katie) katies.get(j);
//            katiej.resetLocks();
//          }
//          break;
//          
//          /// print information
//          case 'i':
//          case 'I':
//          println("Katie ID: "+ katie.id + ", change heading " + katie.change_heading);
//          break;
//          
//          case 'w':
//          case 'W':
//          println("Add waypoint to Katie "+ katie.id);
//          katie.waypoint_add = true;
//          global_waypoint_add = true; // may not be needed
//          for (int j=0; j<katies.size(); j++) {
//            if ( i == j ) continue;
//            katiej = (Katie) katies.get(j);
//            katiej.resetLocks();
//          }
//          break;
//        }
//      }
//    }
//  }  
  
  void resetLocks () {
    Katie katie;
    for (int i=0; i<katies.size(); i++) {
      katie = (Katie) katies.get(i);
      katie.resetLocks();
    }
  }
  
  void update() {
  Katie katie;
  Katie katiej;
  for (int i=0; i<katies.size(); i++) {
      katie = (Katie) katies.get(i);
      //
      for (int j=0; j<katies.size(); j++) {
        if (i == j) continue;
        katiej = (Katie) katies.get(j);
        if (katiej.drag_locked == true) continue;
        katie.calcKatieKatieInfluences((Katie) katies.get(j));
      }
    }
  }
  
  
}


public float getDistance(Vector v1, Vector v2) {
  return sqrt(sq(v1.x-v2.x) + sq(v1.y-v2.y));  
}


///// This function causes problems with javascript
//public void line(Vector v1, Vector v2) {
//  line(v1.x, v1.y, v2.x, v2.y);
//}
//
//
//
////path planning
////int angle, distance;
//
//int[] command_array = new int[0];
//
////int velocity;
//
////headers for different commands
//int HEAD_command_stream = 0xFFFA;
//int HEAD_single_command = 0xFFFB;
//
//void sendResetCommand(int katie_id){
//  sendSingleCommandWithId((byte)0xA1, katie_id); //the command for resetting the katie
//}
//
//void sendSingleCommandWithId(byte command, int katie_id){
//  int[] packet = new int[6];
//  packet[0] = (((byte)HEAD_single_command)& 0xFF);
//  packet[1] = (((byte)HEAD_single_command>>8)& 0xFF);
//  packet[2] = ((byte)(katie_id)& 0xFF);
//  packet[3] = ((byte)(katie_id>>8)& 0xFF);
//  packet[4] = command;
//  packet[5] = ~command; //checksum
//  
//  for (int i = 0; i < 6; i++){
//     g_serial.write((byte)(packet[i])& 0xFF);
//  }
//  
//  print("Single Command with Katie ID: ");
//  print(command);
//  print(" ");
//  println(~command);
//}
//
//void sendCommandPacketWithId(int katie_id){
//  if(command_array.length == 0){
//  return;
//  }
//  print("sending command packet with katie id: ");
//  
//  int number_of_commands = command_array.length;
//  int[] packet = new int[(command_array.length)*2+4+2];
//  
//  //header
//  packet[0] = ((byte)(HEAD_command_stream)& 0xFF);
//  packet[1] = ((byte)(HEAD_command_stream>>8)& 0xFF);
//  
//  //katie id
//  packet[2] = ((byte)(katie_id)& 0xFF);
//  packet[3] = ((byte)(katie_id>>8)& 0xFF);
//  
//  //number of commands ... won't exceed 16 so only one byte is needed
//  packet[4] = ((byte)(number_of_commands)& 0xFF);
//  
//  //commands
//  //start at packet[3], ...no, at packet[5] now!
//  for(int i = 0; i < number_of_commands; i++){
//    packet[i*2+5] = ((byte)(command_array[i])& 0xFF);
//    packet[i*2+6] = ((byte)(command_array[i]>>8)& 0xFF);
//  }
//  
//  //calculate checksum
//  long checksum = 0;
//  for (int j = 4; j < packet.length - 1; j++){ 
//    checksum += packet[j];
//    print(packet[j]);
//    print(" ");
//  }
//  
//  packet[packet.length-1] = (byte)~checksum;
//  print((byte)packet[packet.length-1]);
//  println();
//  
//  
//  //send the packet
//  for (int k = 0; k < packet.length; k++){
//    g_serial.write((byte)(packet[k])& 0xFF);
//    print(packet[k] + " ");
//  }
//  println();
//  clearCommands();
//  
//}
//
//void stackCommand(int angle, int distance, int velocity){
//  //uhg, array stuff...
//  //make a new array sized  +2, then copy in the data
//  int[] next_command_array = new int[command_array.length +3];
//  for (int i = 0; i < command_array.length; i++){
//   next_command_array[i] = command_array[i]; 
//  }
//  //add the next data on the end
//  next_command_array[command_array.length ] = angle;
//  next_command_array[command_array.length +1] = velocity;
//  next_command_array[command_array.length +2] = distance;
//  //now <- next array
//  command_array = next_command_array;
//  
//  //displayCommands();
//}
//
//
//void clearCommands(){
////  angle_knob.setValue(0);
////  distance_slider.setValue(0);
////  velocity_slider.setValue(200);
//  command_array = new int[0];
////  displayCommands();
//  
//}

public class Vector {
  float x, y;
  float a, b;

  public Vector() {
    x = 0;
    y = 0;  
    a = 0;
    b = 0;
  }

  public Vector(float _nx, float _ny) {
    x = _nx;
    y = _ny;  
  }

  public float magnitude() {
    return sqrt(sq(x) + sq(y));  
  }

//  public Vector normalize() {
//    float len = magnitude();
//    if (len < epsilon) return this;
//    return scale(1/len);
//  }

  public Vector add(Vector v) {
    return set(x + v.x, y + v.y); 
  }

  public Vector scale(float f) {
    return set(x*f, y*f);
  } 

  public Vector set(float nx, float ny) {
    x = nx;
    y = ny;

    return this;
  }

  public Vector set(Vector v) {
    x = v.x;
    y = v.y;

    return this;  
  }

  public Vector copy() {
    return new Vector(x,y);  
  }
}



