/* Jordan Jalles
   rHouse Katie project
   data display and saving
*/

// code originally by 
// Maurice Ribble 
// 6-28-2009
// http://www.glacialwanderer.com/hobbyrobotics

// Globals
int g_winW             = 1020;   // Window Width
int g_winH             = 600;   // Window Height          
boolean g_enableFilter = false;  // Enables simple filter to help smooth out data

cDataArray g_piezo    = new cDataArray(200);
cDataArray g_therm    = new cDataArray(200);
cDataArray g_light    = new cDataArray(200);
cDataArray g_rear_range      = new cDataArray(200);
cDataArray g_therm1     = new cDataArray(200);
cDataArray g_therm2 = new cDataArray(200);

cDataArray g_rbump = new cDataArray(200);
cDataArray g_lbump = new cDataArray(200);
cDataArray g_wheel = new cDataArray(200);
cDataArray g_createD1 = new cDataArray(200);
cDataArray g_createD2 = new cDataArray(200);
cDataArray g_createD3 = new cDataArray(200);
cDataArray g_createD4 = new cDataArray(200);
cDataArray g_createD5 = new cDataArray(200);

cGraph g_graph_heat = new cGraph(410, 370, 400, 200);
cGraph g_graph_light = new cGraph(410, 70, 400, 50);
cGraph g_graph_piezo = new cGraph(410, 150, 400, 50);
cGraph g_graph_distance = new cGraph(410, 260, 400, 50);
cGraph g_graph_create_distance = new cGraph(10, 70, 390, 120);
cGraph g_graph_lbump = new cGraph(10, 270, 390, 10);
cGraph g_graph_rbump = new cGraph(10, 240, 390, 10);
cGraph g_graph_wheel = new cGraph(10, 210, 390, 10);

void setup()
{
  size(g_winW, g_winH, P2D);
  textSize(14);
  //noLoop();
} 

void draw()
{
  //draw the background color
  fill(100, 100, 100);
  stroke(100, 100, 100);
  rect(0, 0, g_winW, g_winH);
  
  //draw the heat graph key
  strokeWeight(2.5);
  fill(0, 0, 0);
//  stroke(255, 0, 0);     line(420, 240, 435, 240);
//  text("thermistor", 440, 250);
  stroke(255, 0, 255);   line(420, 240, 435, 240);
  text("IR thermometer 1", 440, 250);
  stroke(100, 255, 100); line (560, 240, 575, 240);
  text("IR thermometer 2", 580, 250);

  
    //draw the distance sensor graph key
  strokeWeight(2.5);
  fill(0, 0, 0);
//  stroke(255, 0, 0);     line(420, 240, 435, 240);
//  text("thermistor", 440, 250);
  stroke(0, 0, 255);
  line(420, 440, 435, 440);
  text("Rear Distance Sensor", 440, 450);
  stroke(255, 0, 0); line (420, 460, 435, 460);
  text("Upward Distance Sensor", 440, 470);
  
  
  //camera frame image
  stroke(0,0,0, 0);
  fill(0);
  rect(10, 30, 400, 300);
  fill(255, 255,255, 120);
  text("axis image frame", 100, 100);
  
  text("Time Now: " + getTimeString(), 500, 595);
  text("FPS: " + (int)frameRate, 20, 595);
  
  //draw the graphs
  strokeWeight(1);
  fill(0);
  g_graph_heat.drawGraphBox();
  g_graph_light.drawGraphBox();
  g_graph_piezo.drawGraphBox();
  g_graph_distance.drawGraphBox();
  g_graph_create_distance.drawGraphBox();
  g_graph_lbump.drawGraphBox();
  g_graph_rbump.drawGraphBox();
  g_graph_wheel.drawGraphBox();
  
  //label the heat graph
  label_graph(g_graph_heat,"Heat: Degrees Farenheit", 0, 120, 20);
  label_graph(g_graph_light,"Light");
  label_graph(g_graph_distance,"Distance");
  label_graph(g_graph_create_distance,"Create Cliff/Wall Sensors");
  label_graph(g_graph_lbump,"Left Bump");
  label_graph(g_graph_rbump,"Right Bump");
  label_graph(g_graph_wheel,"Wheels Down");
  label_graph(g_graph_piezo,"Vibration");
  
  
  strokeWeight(1);
  stroke(0, 255, 0);
  g_graph_piezo.drawLine(g_piezo, 0, 1024);
  stroke(0, 255, 255);
  g_graph_light.drawLine(g_light, 0, 1024);
  stroke(0, 0, 255);
  g_graph_distance.drawLine(g_rear_range, 0, 1024);
  stroke(255, 0, 255);
  g_graph_heat.drawLine(g_therm1, 0, 120);
  stroke(255, 0, 0);
  g_graph_distance.drawLine(g_therm, 0, 1024); /// changed for up distance
  stroke(100, 255,100);
  g_graph_heat.drawLine(g_therm2, 0, 120);
  stroke(255, 0, 0);
  g_graph_lbump.drawLine(g_lbump, 0, 1);
  g_graph_rbump.drawLine(g_rbump, 0, 1);
  g_graph_wheel.drawLine(g_wheel, 0, 1);
  
  stroke(0, 200, 200);
  g_graph_create_distance.drawLine(g_createD1, 0, 100);
  stroke(200, 0, 200);
  g_graph_create_distance.drawLine(g_createD2, 0, 1200);
  g_graph_create_distance.drawLine(g_createD3, 0, 1200);
  g_graph_create_distance.drawLine(g_createD4, 0, 1200);
  g_graph_create_distance.drawLine(g_createD5, 0, 1200);  
}

void label_graph(cGraph graph, String name){
 label_graph(graph, name, 0,0,10); 
}

void label_graph(cGraph graph, String name, int min, int max, int increment){
  int numLabels = (max-min)/increment;
  strokeWeight(.5);
  stroke(150);
  fill(0);
  text(name, graph.m_gLeft, graph.m_gTop-2);
  
  fill(200);
  textSize(9);
  for (int i = 0; i <= numLabels; i++){
    float y_loc = graph.m_gBottom - (i*graph.m_gHeight/numLabels);
    text(increment*i, graph.m_gRight, y_loc +5);
    line(graph.m_gLeft, y_loc, graph.m_gRight, y_loc); 
  }
  textSize(14);
}

// This pushes one set of the data received from the websocket
void pushData()
{
    g_piezo.addVal(piezo);
    g_therm.addVal(therm);
    g_light.addVal(light);
    g_rear_range.addVal(rear_range);
    g_therm1.addVal(ir_therm1);
    g_therm2.addVal(ir_therm2);
    if (bump_wheel > 3){
      g_wheel.addVal(1);
      
      bump_wheel = 0;
    }else{
       g_wheel.addVal(0);
    }
    g_rbump.addVal((bump_wheel%2));
    g_lbump.addVal(bump_wheel > 1 ? 1:0);
    
    //add create distance values
    if (cd1 > 100){
     cd1 = 0; 
    }
    if (cd2 > 1200){
     cd2 = 0; 
    }
    if (cd3 > 1200){
     cd3 = 0; 
    }
    if (cd4 > 1200){
     cd4 = 0; 
    }
    if (cd5 > 1200){
     cd5 = 0; 
    }
    g_createD1.addVal(cd1);
    g_createD2.addVal(cd2);
    g_createD3.addVal(cd3);
    g_createD4.addVal(cd4);
    g_createD5.addVal(cd5);

    //redraw();
  }

// This class helps mangage the arrays of data I need to keep around for graphing.
class cDataArray
{
  float[] m_data;
  int m_maxSize;
  int m_startIndex = 0;
  int m_endIndex = 0;
  int m_curSize;
  
  cDataArray(int maxSize)
  {
    m_maxSize = maxSize;
    m_data = new float[maxSize];
  }
  
  void addVal(float val)
  {
    
    if (g_enableFilter && (m_curSize != 0))
    {
      int indx;
      
      if (m_endIndex == 0)
        indx = m_maxSize-1;
      else
        indx = m_endIndex - 1;
      
      m_data[m_endIndex] = getVal(indx)*.5 + val*.5;
    }
    else
    {
      m_data[m_endIndex] = val;
    }
    
    m_endIndex = (m_endIndex+1)%m_maxSize;
    if (m_curSize == m_maxSize)
    {
      m_startIndex = (m_startIndex+1)%m_maxSize;
    }
    else
    {
      m_curSize++;
    }
  }
  
  float getVal(int index)
  {
    return m_data[(m_startIndex+index)%m_maxSize];
  }
  
  int getCurSize()
  {
    return m_curSize;
  }
  
  int getMaxSize()
  {
    return m_maxSize;
  }
}

// This class takes the data and helps graph it
class cGraph
{
  float m_gWidth, m_gHeight;
  float m_gLeft, m_gBottom, m_gRight, m_gTop;
  
  cGraph(float x, float y, float w, float h)
  {
    m_gWidth     = w;
    m_gHeight    = h;
    m_gLeft      = x;
    m_gBottom    = g_winH - y;
    m_gRight     = x + w;
    m_gTop       = g_winH - y - h;
  }
  
  void drawGraphBox()
  {
    stroke(0, 0, 0);
    rectMode(CORNERS);
    rect(m_gLeft, m_gBottom, m_gRight, m_gTop);
  }
  
  void drawLine(cDataArray data, float minRange, float maxRange)
  {
    float graphMultX = m_gWidth/data.getMaxSize();
    float graphMultY = m_gHeight/(maxRange-minRange);
    if(minRange == 0 && maxRange == 1){
       for(int i=0; i<data.getCurSize()-1; i++)
       {
        float dw = (i-100)/100.0;
        strokeWeight((data.getVal(i)+data.getVal(i+1))/2*m_gHeight + 1);
        float x0 = i*graphMultX+m_gLeft;
        float y0 = m_gBottom-(m_gHeight)/2;
        float x1 = (i+1)*graphMultX+m_gLeft;
        float y1 = m_gBottom-(m_gHeight)/2;
        line(x0, y0, x1, y1);
      }
      
    }else{
      for(int i=0; i<data.getCurSize()-1; i++)
      {  
        float dw = (i-100)/100.0;
        strokeWeight(2-1.5*(dw)*(dw));//*(dw)*(dw));
        //strokeWeight(1);
        float x0 = i*graphMultX+m_gLeft;
        float y0 = m_gBottom-((data.getVal(i)-minRange)*graphMultY);
        float x1 = (i+1)*graphMultX+m_gLeft;
        float y1 = m_gBottom-((data.getVal(i+1)-minRange)*graphMultY);
        line(x0, y0, x1, y1);
      }
    }
  }
}




