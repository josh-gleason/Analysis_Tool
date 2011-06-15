////////////////////////////////////////////////////////////////////////
// results.cc
// author: Ara V Nefian
// NASA Ames Research Center
////////////////////////////////////////////////////////////////////////
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <cv.h>
#include <highgui.h>
#include "ptd_defs.h"
#include "hungarian.h"
#include "analysis_tools.h"

// comment this out to disable drawing results (saves a lot of time)
//#define DRAW_RESULTS

#define OVERLAP_SCORE

// any occurances of %s in the filenames will be replaced by argv[1]

// input
#define TRUE_ROI_FNAME        "../../../../data_local/%s_imgs_roi.txt"
#define COMP_ROI_FNAME        "../../../../results/%s_imgs_res.txt"

// output
#define OUT_ROI_FNAME         "../results/%s_imgs_res_temp.txt"
#define FA_TP_RESULTS_FNAME   "../results/fa_tp_%s_res.txt"
#define PR_RE_RESULTS_FNAME   "../results/pr_re_%s_res.txt"
#define DRAW_RESULTS_FOLDER   "../results/%s"

// old values
//#define FA_TP_RESULTS_FNAME   "../../../../results/fa_tp_results.txt"
//#define PR_RE_RESULTS_FNAME   "../../../../results/pr_re_results.txt"
//#define DRAW_RESULTS_FOLDER   "../results/2009_07_23"

#define TEX_RPL(a) FindReplace(a,"%s",argv[1]).c_str()

using namespace std;

// replace all occurance of <repl> in <input> with <replValue>
string FindReplace( const string& input, const string& repl, const string& replValue )
{
  string retVal = input;
  size_t loc, len;
  len = repl.size();
  
  loc = retVal.find(repl);
  while ( loc != string::npos )
  {
    retVal = retVal.replace(loc,len,replValue);
    loc = retVal.find(repl);
  }

  return retVal;
}

void DrawResults(IplImage *img, int numTrueROI, rect* trueROIArray, 
    int numCompROI, rect *compROIArray, 
    char *outImgFilename, 
    float scoreDetectionThreshold, float spaceDetectionThreshold, 
    int *matchingIndex, float *matchingDist)
{
  int i;
  CvPoint pt1, pt2; 

  //true ROI 
  for (i = 0; i < numTrueROI; i++){
    rect this_roi = trueROIArray[i];
    pt1.x = this_roi.xl;
    pt2.x = this_roi.xr; 
    pt1.y = this_roi.yt;
    pt2.y = this_roi.yb;

    cvRectangle(img, pt1, pt2, CV_RGB(0,255,0), 3, 8, 0 );
  }  

  //computed ROI
  for (i = 0; i < numCompROI; i++){
    rect this_roi = compROIArray[i];
    pt1.x = this_roi.xl;
    pt2.x = this_roi.xr; 
    pt1.y = this_roi.yt;
    pt2.y = this_roi.yb;

    //printf("Rectangle : (%d, %d) (%d, %d)\n",pt1.x,pt1.y,pt2.x,pt2.y);
    
    if (this_roi.colorScore > scoreDetectionThreshold)
      cvRectangle(img, pt1, pt2, CV_RGB(255, 0, 0), 3, 8, 0 );
  }

  if ( numCompROI > 0 )
    for (i = 0; i < numTrueROI; i++){
      rect this_roi = trueROIArray[i];

      pt1.x = (int)floor(0.5*(this_roi.xl + this_roi.xr));
      pt1.y = (int)floor(0.5*(this_roi.yt + this_roi.yb));

      int j = matchingIndex[i];

      if ( j < numCompROI && j >= 0 )
      {
        rect comp_roi = compROIArray[j];
        pt2.x = (int)floor(0.5*(comp_roi.xl + comp_roi.xr));
        pt2.y = (int)floor(0.5*(comp_roi.yt + comp_roi.yb));

        //display lines for matches that are below a fixed threshold
        if (matchingDist[i] < spaceDetectionThreshold){
          cvLine( img, pt1, pt2, CV_RGB(0,255,0), 3);
        }
      }
    }  

  cvSaveImage( outImgFilename, img);
}

//saves all the detected patches and assigns them to target or ackground based on 
//their relative position to the true target
void SaveResults(const char *resFilename, const char *imgFilename, 
    int numTrueROI, rect* trueROIArray, 
    int numCompROI, rect *compROIArray,  
    float scoreDetectionThreshold, float spaceDetectionThreshold, 
    int *matchingIndex, float *matchingDist)
{
  int i, k;

  //TO DO: determine the number of patterns above the threshold

  FILE *fp = fopen(resFilename, "a");
  int numPatterns = 0;   
  for (k = 0; k < numCompROI; k++){
    if (compROIArray[k].colorScore > scoreDetectionThreshold){ //only selected patterns
      numPatterns++;
    }
  }
  fprintf(fp, "%s %d ", imgFilename, numPatterns);

  for (k = 0; k < numCompROI; k++){

    if (compROIArray[k].colorScore > scoreDetectionThreshold){ //only selected patterns

      int target = 0;

      for (i = 0; i < numTrueROI; i++){
        if ((matchingIndex[i] == k) && (matchingDist[i] < spaceDetectionThreshold)){
          target = 1;
        }
      }

      if (target == 1){
        fprintf (fp, ": vehicle %d %d %d %d ", compROIArray[k].xl, compROIArray[k].yt, compROIArray[k].xr, compROIArray[k].yb); 
      }
      else{
        fprintf (fp, ": background %d %d %d %d ", compROIArray[k].xl, compROIArray[k].yt, compROIArray[k].xr, compROIArray[k].yb); 
      }

    }

  }   

  fprintf(fp, "\n");
  fclose(fp);
}


void DisplayPRCurve(int**falseAlarms, int**trueDetections, int numSpaceDetectThreshLevels, int numScoreDetectThreshLevels)
{
  IplImage *falseAlarmRateImg;
  int prImgWidth = 400;
  int prImgHeight = 400;
  int prImgChannels = 3;   
  CvFont font;
  double hScale=0.3;
  double vScale=0.3;
  int    lineWidth=1; 

  falseAlarmRateImg = cvCreateImage( cvSize(prImgWidth, prImgHeight), 8, 3);
  int  step = falseAlarmRateImg->widthStep;

  for(int i=0;i<prImgHeight;i++){ 
    for(int j=0;j<prImgWidth;j++){ 
      for(int k=0;k<prImgChannels;k++){
        ((uchar *)falseAlarmRateImg->imageData)[i*step+j*prImgChannels+k]=255;
      }
    }
  }


  for (int l = 0; l< numSpaceDetectThreshLevels; l++){
    for (int s = 0; s< numScoreDetectThreshLevels-1; s++){
      printf("falseAlarmRate[%d][%d] = %d trueDetectionRate[%d][%d] = %d\n", s, l, falseAlarms[s][l], s, l, trueDetections[s][l]);
      cvLine(falseAlarmRateImg, cvPoint(falseAlarms[s][l]*40, trueDetections[s][l]*40), cvPoint(falseAlarms[s+1][l]*40, trueDetections[s+1][l]*40), 
          cvScalar(255, 0, 0), 1);
    }
  }

  cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
  cvPutText (falseAlarmRateImg,"Precision recall",cvPoint(20,20), &font, cvScalar(255,255,0));

  cvNamedWindow("win1", CV_WINDOW_AUTOSIZE); 
  cvShowImage("win1",  falseAlarmRateImg);
  cvWaitKey(0);

  cvSaveImage("Prec-recall.jpg", falseAlarmRateImg);

  cvReleaseImage(&falseAlarmRateImg);
}


void SaveResults(int**falseAlarms, int**trueDetections, int totalTrueROI, int numSpaceDetectThreshLevels, int numScoreDetectThreshLevels, string arg)
{

  FILE *fp;
  fp = fopen(FindReplace(FA_TP_RESULTS_FNAME,"%s",arg).c_str(), "w");
  for (int l = 0; l< numSpaceDetectThreshLevels; l++){
    for (int s = 0; s< numScoreDetectThreshLevels; s++){
      fprintf(fp, "s = %d l = %d falseAlarms= %d trueDetectionRate= %f\n", s, l, falseAlarms[s][l], trueDetections[s][l]/(float)totalTrueROI);
    }
  }
  fclose(fp);

  fp = fopen(FindReplace(PR_RE_RESULTS_FNAME,"%s",arg).c_str(), "w");
  for (int l = 0; l< numSpaceDetectThreshLevels; l++){
    for (int s = 0; s< numScoreDetectThreshLevels; s++){
      fprintf(fp, "s = %d l = %d precision= %f recall= %f\n", 
          s, l, trueDetections[s][l]/(float)(trueDetections[s][l]+falseAlarms[s][l]), 
          trueDetections[s][l]/(float)totalTrueROI);
    }
  }
  fclose(fp);

}
void ReadResults(int**falseAlarms, int**trueDetections, int numSpaceDetectThreshLevels, int numScoreDetectThreshLevels)
{
  int t_s, t_l, t_falseAlarms, t_trueDetections;
  int MAX_LENGTH = 5000;
  char line[MAX_LENGTH];
  // FILE *fp;
  //fp = fopen("pr_results.txt", "r");
  std::ifstream fp("pr_results.txt");

  for (int l = 0; l< numSpaceDetectThreshLevels; l++){
    for (int s = 0; s< numScoreDetectThreshLevels; s++){
      fp.getline(line, MAX_LENGTH);
      sscanf(line, "s = %d l = %d falseAlarms = %d trueDetections = %d", &t_s, &t_l, &t_falseAlarms, &t_trueDetections);
      falseAlarms[s][l] = falseAlarms[s][l] + t_falseAlarms;
      trueDetections[s][l] = trueDetections[s][l] + t_trueDetections;
    }
  }
  fp.close();
}


rect* ReadTrueLine(char line[], int *numTrueROI, char *imgFilename)
{
  int i;
  //char *imgFilename;
  char label[200];
  int   l_numTrueROI;
  int   w, h;
  rect *roiArray = NULL;

  //imgFilename = new char[500];

  sscanf(line, "%s %d", imgFilename, &l_numTrueROI);  
  //printf("true_filename = %s\n", imgFilename);

  //printf ("l_numTrueROI = %d\n", l_numTrueROI);

  if (l_numTrueROI == 0){
    *numTrueROI = l_numTrueROI;
    return roiArray;
  }

  roiArray = new rect[l_numTrueROI];

  char * pch;
  pch=strchr(line,':');
  printf("pch = %s\n", pch);
  i = 0;
  while (pch!=NULL)
  {
    printf ("found at %d\n",pch-line+1);
//    label = new char[200];
    label[0] = '\0';  // clear string
    sscanf(pch, ": %s %d %d %d %d", label, &(roiArray[i].xl), &(roiArray[i].yt), &w, &h);

    roiArray[i].xr = roiArray[i].xl + w;
    roiArray[i].yb = roiArray[i].yt + h;

    printf("xl = %d, yt = %d, xr = %d, yb = %d\n", 
    	     roiArray[i].xl, roiArray[i].yt, roiArray[i].xr, roiArray[i].yb);

    pch=strchr(pch+1,':');
    printf("pch = %s\n", pch);
    i++;
//    delete label;
  }

  *numTrueROI = l_numTrueROI;
  return roiArray;

}

rect* ReadCompLine(char line[], int *numCompROI, char *imgFilename)
{
  int i;
  //char *imgFilename;
  char label[200];
  int l_numCompROI;
  rect *roiArray = NULL;

  //imgFilename = new char[500];

  sscanf(line, "%s %d", imgFilename, &l_numCompROI);

  //printf("comp_filename = %s\n", imgFilename);
  *numCompROI = l_numCompROI;

  //printf("numCompROI = %d\n", l_numCompROI);

  if (l_numCompROI == 0){
    return roiArray;
  }

  roiArray = new rect[l_numCompROI];

  char * pch;
  pch=strchr(line,':');
  //printf("pch = %s\n", pch);
  i = 0;
  while (pch!=NULL)
  {
    //printf ("found at %d\n",pch-line+1);
//    label = new char[200];
    label[0] = '\0';
    sscanf(pch, ": %s %f %f %d %d %d %d", 
        label, &(roiArray[i].score), &(roiArray[i].colorScore), &(roiArray[i].xl), &(roiArray[i].yt), &(roiArray[i].xr), &(roiArray[i].yb));
    //printf("xl = %d, yt = %d, xr = %d, yb = %d, score = %f, colorScore = %f\n", 
    //	     roiArray[i].xl, roiArray[i].yt, roiArray[i].xr, roiArray[i].yb, roiArray[i].score, roiArray[i].colorScore);
    pch=strchr(pch+1,':');
    //printf("pch = %s\n", pch);
    i++;
  }

  //delete imgFilename;

  return roiArray;

}


float ROIdistance(rect trueROI, rect compROI)
{
#ifdef OVERLAP_SCORE 
  at::Rect tROI(trueROI.xl,trueROI.yt,trueROI.xr-trueROI.xl,trueROI.yb-trueROI.yt); 
  at::Rect cROI(compROI.xl,compROI.yt,compROI.xr-compROI.xl,compROI.yb-compROI.yt); 

  // make a higher score a better score (prevents me from needing to change
  // the rest of the code)
  return 1.0-at::computeScore(tROI,cROI);
#else // use old distance score
  int roi_width = compROI.xr - compROI.xl;
  int roi_height = compROI.yb - compROI.yt;

  float d_xl = fabs(compROI.xl - trueROI.xl)/(float)roi_width;
  float d_xr = fabs(compROI.xr - trueROI.xr)/(float)roi_width;
  float d_yt = fabs(compROI.yt - trueROI.yt)/(float)roi_height;
  float d_yb = fabs(compROI.yb - trueROI.yb)/(float)roi_height;

  float d  = (d_xl + d_xr + d_yt + d_yb)/4;

  return d;
#endif
}

void SelectPatterns(rect *compROI, int numCompROI,  CvSeq* t_compROI, float threshold)
{
  for (int k = 0; k < numCompROI; k++){
    if (compROI[k].colorScore > threshold){ //pick the detection for these threshold values
      rect comp;
      comp.xl = compROI[k].xl;
      comp.yt = compROI[k].yt;
      comp.xr = compROI[k].xr;
      comp.yb = compROI[k].yb;
      comp.score = compROI[k].score;
      comp.index = k;
      cvSeqPush(t_compROI, &comp );
    }
  }
}


void GetAllMatches_Direct(rect *trueROI, int numTrueROI, CvSeq* t_compROI, int *matchingIndex, float *matchingDist)
{
  int t_numCompROI = t_compROI->total;

  for (int i = 0; i < numTrueROI; i++){

    matchingDist[i] = 1000000000.0;

    for (int j = 0; j < t_numCompROI; j++){
      rect compROI = *(rect*)cvGetSeqElem(t_compROI, j );
      float dist = ROIdistance(trueROI[i], compROI);
      if (dist < matchingDist[i]){
        matchingIndex[i] = compROI.index;//j;
        matchingDist[i] = dist;
      }
    }    
  }


}
//compute the global matches between the true and computed detections   
//functions assumes that both the number of true and computed patterns are non zero
void GetAllMatches(rect *trueROI, int numTrueROI, CvSeq* t_compROI, int *matchingIndex, float *matchingDist)
{
  int t_numCompROI = t_compROI->total;

  if ((numTrueROI != 0) && (t_numCompROI != 0)){
    int* r;
    r = (int*)malloc(sizeof(int)*numTrueROI*t_numCompROI);

    for (int i = 0; i < numTrueROI; i++){
      for (int j = 0; j < t_numCompROI; j++){

        rect compROI = *(rect*)cvGetSeqElem(t_compROI, j );
        r[i*numTrueROI+j] = (int)ROIdistance(trueROI[i], compROI);
      }
    }

    printf("numTrueROI = %d, numCompROI = %d\n", numTrueROI, t_numCompROI);
    printf("before\n");
    hungarian_t prob;
    hungarian_init(&prob,(int*)r,numTrueROI,t_numCompROI);
    hungarian_print_rating(&prob);
    hungarian_solve(&prob);
    hungarian_print_assignment(&prob);
    printf("after\n");

    printf("\nfeasible? %s\n", 
        (hungarian_check_feasibility(&prob)) ? "yes" : "no");
    printf("benefit: %d\n\n", hungarian_benefit(&prob));

    for (int i = 0; i < numTrueROI; i++){
      matchingIndex[i] = prob.a[i];
      matchingDist[i]= r[i*numTrueROI + matchingIndex[i]];
    }
  }
}


//select only the computed targets that are within a spaceThreshold distance from the best matched true targets  
void GetGoodMatches(int *matchingIndex, float* matchingDist,  int numTrueROI, int numCompROI, 
    float spaceThreshold, int *falseAlarms, int *trueDetections, int *falseAlarmsImgs)
{

  int l_falseAlarmsImg = 0; 
  int l_falseAlarms = 0;
  int l_trueDetections = 0;

  if ((numTrueROI == 0) && (numCompROI != 0)){
    // all detected patterns are false alarms
    l_falseAlarms = numCompROI;
    l_falseAlarmsImg = 1;
  }

  if ((numTrueROI != 0 ) && (numCompROI != 0)){

    if (numTrueROI > numCompROI){
      l_falseAlarms = numCompROI - numTrueROI; 
      l_falseAlarmsImg = 1;
    }

    for (int i = 0; i < numTrueROI; i++){

      float distance = matchingDist[i];
      if (distance < spaceThreshold){
        l_trueDetections = l_trueDetections + 1;
      }
      else{
        l_falseAlarms = l_falseAlarms + 1;
        l_falseAlarmsImg = 1;
      } 
    }  
  }

  *falseAlarms = l_falseAlarms;

  *falseAlarmsImgs = l_falseAlarmsImg;

  *trueDetections = l_trueDetections;
}

void ROIMatch(rect *trueROI, int numTrueROI, rect *compROI, int numCompROI, 
    int numScoreDetectThreshLevels, int numSpaceDetectThreshLevels, 
    float *scoreDetectThreshLevels, float *spaceDetectThreshLevels,
    int colorScoreThresh, int **falseAlarms, int **falseAlarmsImg, 
    int **trueDetections, int **matchingIndex, float **matchingDist)
{


  for (int s = 0; s < numScoreDetectThreshLevels; s++){

    CvMemStorage* t_roi_storage = cvCreateMemStorage(0);
    CvSeq* t_compROI = cvCreateSeq( 0, sizeof(CvSeq), sizeof(rect), t_roi_storage);

    //step 1: select surving computed detection 
    SelectPatterns(compROI, numCompROI,  t_compROI, scoreDetectThreshLevels[s]);

    //float *l_matchingDist = new float[numTrueROI];

    //step 2: compute the global matches between the tru detection and surviving detections   
    //GetAllMatches(trueROI, numTrueROI, t_compROI, matchingIndex[s], l_matchingDist);
    GetAllMatches_Direct(trueROI, numTrueROI, t_compROI, matchingIndex[s], /*l_matchingDist*/matchingDist[s]);

    //step 3: select the surving matches and update the results  
    for (int l = 0; l < numSpaceDetectThreshLevels; l++){

      int l_falseAlarms = 0;
      int l_falseAlarmsImg = 0;
      int l_trueDetections = 0 ; 

      GetGoodMatches(matchingIndex[s], /*l_matchingDist*/matchingDist[s], numTrueROI, t_compROI->total,  spaceDetectThreshLevels[l], 
          &l_falseAlarms, &l_trueDetections, &l_falseAlarmsImg);

      falseAlarms[s][l] = falseAlarms[s][l] + l_falseAlarms;
      falseAlarmsImg[s][l] = falseAlarmsImg[s][l] + l_falseAlarmsImg;
      trueDetections[s][l] = trueDetections[s][l] + l_trueDetections;
    }

    //release memory 
    //delete l_matchingDist; 
    cvReleaseMemStorage(&t_roi_storage);

  }

}


int main (int argc, char *argv[])
{

  if(argc<2){
    printf("Usage: results <image-database-name>\n\7");
    exit(0);
  }
  // ====================== INITIALIZATION VARIABLES ==========================
  int MAX_LENGTH = 5000;

  char trueLabelLine[MAX_LENGTH];
  char compLabelLine[MAX_LENGTH];

  int numTrueROI;
  int numCompROI;
  rect *trueROIArray;
  rect *compROIArray;

  int numScoreDetectThreshLevels = 10;  // TODO : change this to 1
#ifdef OVERLAP_SCORE
  int numSpaceDetectThreshLevels = 21;  // 0 to 1.0 
#else
  int numSpaceDetectThreshLevels = 13;
#endif
  float *scoreDetectThreshLevels = new float[numScoreDetectThreshLevels];
  float *spaceDetectThreshLevels = new float[numSpaceDetectThreshLevels];

  for (int i = 0; i < numSpaceDetectThreshLevels; i++){
    //spaceDetectThreshLevels[i] = 1.0 + 0.05*i;
#ifdef OVERLAP_SCORE
    spaceDetectThreshLevels[i] = 0.0 + 0.05*i;
#else
    spaceDetectThreshLevels[i] = 0.0 + 0.25*i;
#endif
  }
  for (int i = 0; i < numScoreDetectThreshLevels; i++){
    //scoreDetectThreshLevels[i] = 20000 + i*5000;
    scoreDetectThreshLevels[i] = 200 + 10*i;
  }
  int colorScoreThresh = 300;//1;

  int **falseAlarms = new int*[numScoreDetectThreshLevels];
  int **falseAlarmsImg = new int*[numScoreDetectThreshLevels];
  int **trueDetections = new int*[numSpaceDetectThreshLevels];
  for (int s = 0; s < numScoreDetectThreshLevels; s++){
    falseAlarms[s] = new int[numSpaceDetectThreshLevels];
    falseAlarmsImg[s] = new int[numSpaceDetectThreshLevels];
    trueDetections[s] = new int[numSpaceDetectThreshLevels];
    for ( int j = 0; j < numSpaceDetectThreshLevels; j++ )
    {
      falseAlarms[s][j] = 0;
      falseAlarmsImg[s][j] = 0;
      trueDetections[s][j] = 0;
    }
  }

  int totalTrueROI = 0;
  // ====================== INITIALIZATION VARIABLES ==========================

  for (int i = 1; i < argc; i++){
    std::ifstream trueLabelListFile(TEX_RPL(TRUE_ROI_FNAME));
    std::ifstream compLabelListFile(TEX_RPL(COMP_ROI_FNAME));

    int temp_index = 0;
    while (trueLabelListFile.getline(trueLabelLine, MAX_LENGTH) && (compLabelListFile.getline(compLabelLine, MAX_LENGTH))) {

      cout << "Index = " << temp_index << '\r' << flush;
      //printf("test 0\n");

      //read the label files 
      char t_imgFilename[500];
      trueROIArray = ReadTrueLine(trueLabelLine, &numTrueROI, t_imgFilename);
      printf("index = %d, true_filename = %s\n", temp_index, t_imgFilename);

      char c_imgFilename[500]; 
      compROIArray = ReadCompLine(compLabelLine, &numCompROI, c_imgFilename);
      printf("index = %d, comp_filename = %s\n", temp_index, c_imgFilename);
      printf("numTrueROI = %d\n", numTrueROI);
      int **matchingIndex;
      float **matchingDist;

      matchingIndex = new int *[numScoreDetectThreshLevels];
      matchingDist = new float *[numSpaceDetectThreshLevels];

      if (numTrueROI != 0){
        for (int s = 0; s < numScoreDetectThreshLevels; s++){
          matchingIndex[s] = new int[numTrueROI];
        }

        for (int s = 0; s < numSpaceDetectThreshLevels; s++){
          matchingDist[s]  = new float[numTrueROI];
        }
      }

      //match true and comp structures
      ROIMatch (trueROIArray, numTrueROI, compROIArray, numCompROI, 
          numScoreDetectThreshLevels, numSpaceDetectThreshLevels, 
          scoreDetectThreshLevels, spaceDetectThreshLevels, colorScoreThresh,
          falseAlarms, falseAlarmsImg, trueDetections, matchingIndex, matchingDist);

      totalTrueROI = totalTrueROI + numTrueROI; 

      SaveResults(TEX_RPL(OUT_ROI_FNAME), t_imgFilename, 
          numTrueROI, trueROIArray, 
          numCompROI, compROIArray,  
          scoreDetectThreshLevels[9], spaceDetectThreshLevels[4], 
          matchingIndex[9], matchingDist[9]);          

#ifdef DRAW_RESULTS

      IplImage *img;
      img = cvLoadImage(t_imgFilename);
      char outImgFilename[500];
      sprintf (outImgFilename, "%s/comp_res_%s", TEX_RPL(DRAW_RESULTS_FOLDER), strrchr(t_imgFilename, '/') + 1);
      printf("outImgFilename = %s\n", outImgFilename);

      DrawResults(img, 
          numTrueROI, trueROIArray, 
          numCompROI, compROIArray, 
          outImgFilename, 
          scoreDetectThreshLevels[9], spaceDetectThreshLevels[4],
          matchingIndex[9], matchingDist[9]);

      printf("Releasing image\n");
      cvReleaseImage(&img);

#endif // DRAW_RESULTS

      delete matchingIndex;
      delete matchingDist;

      temp_index++;
    }

    trueLabelListFile.close();
    compLabelListFile.close();
  }

  cout << endl;

  SaveResults(falseAlarms, trueDetections, totalTrueROI, numSpaceDetectThreshLevels, numScoreDetectThreshLevels, argv[1]);
  //display the TD/FA for all scoreDetection threshold levels and the most "optimistic" distance threshold 
  for (int l = 0; l <numSpaceDetectThreshLevels; l++){
    printf("distThreshold=%f\n", spaceDetectThreshLevels[l]);
    for (int s = 0; s< numScoreDetectThreshLevels; s++){
      printf("TD[%d][%d] = %f, FA[%d][%d] = %d, FAI[%d][%d]= %d,  scoreDetectThreshLevels[%d]=%f\n", 
          s, l, trueDetections[s][l]/(float)totalTrueROI, 
          s, l, falseAlarms[s][l],
          s, l, falseAlarmsImg[s][l],
          s, scoreDetectThreshLevels[s]);
    }
  }

  //delete memory
  for (int s = 0; s < numScoreDetectThreshLevels; s++){
    delete falseAlarms[s];
    delete trueDetections[s];
  }

  delete falseAlarms;
  delete trueDetections;
  delete scoreDetectThreshLevels;
  delete spaceDetectThreshLevels;

  return 0;
}

