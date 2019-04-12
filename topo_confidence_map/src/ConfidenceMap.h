#ifndef CONFIDENCEMAP_H 
#define CONFIDENCEMAP_H 
//#include "HausdorffMeasure.h"
//flann based

#include <pcl/kdtree/impl/kdtree_flann.hpp>
//#include "GHPR.h"

#include "ExtendedGridMap.h"

#include <stdlib.h>
#include <time.h> 


///************************************************************************///
// a class to implement the Gaussian Process Regression algorithm
// created and edited by Huang Pengdi

//Version 1.1 2018.11.25
// - add the 2D map funtion to organize point cloud storage
//Version 1.2 2018.12.13
// - change all calculation under the grid model
// - modify distance features
//Version 1.3 2018.12.14
// - add noting
// - complete distance term
//version 2.0 2019.04.12
// - real time processing modification
///************************************************************************///


namespace topology_map {

//status

//************details of grid label value in m_vMapGridLabel
// cover means rewrite if new sematic object appears in this grid
// 0 nothing or unknown (0 is covered by 1)
// 1 obstacles (1 is covered by 2)
// 2 ground points (2 is covered by 3)
// 3 boundary 
// for example, boundary grid can not be defined as another classification, but obstacle can be recovered by ground or obstacles 
// the "-" negetive means the grid has been computed in node generation

//************details of grid status value in m_vMapGridTravel
//status indicates whether the gird is travelable or not (can the robot reaches this grid right now)
// -1 indicates it is an unknown grid
// 0 indicates this grid is ground but not reachable now
// 1 indicates this grid is a travelable region grid
// 2 is the new scanned grids (input) without growing
// 3 indicates the grid has been computed
// 4 indicates this grid is a off groud grid (not reachable forever)
struct ConfidenceValue{

	//distance based term
	float travelTerm;
	//visibility based term
	float boundTerm;
	//bound term
	float visiTerm;
	//quality
	float qualTerm;
	//quality
	float totalValue;

	//labels of obstacle, travelable region and boundary
	//0 nothing 
	//1 obstacles
	//2 ground points
	//3 boundary
	short label;

	//travelable or not (can the robot reaches this grid right now)
	//-1 indicates it is an unknown grid
	//0 indicates this grid is ground but not reachable now
	//1 indicates this grid is a travelable region grid
	//2 is the new scanned grids (input) without growing
	//3 indicates the grid has been computed
	//4 indicates this grid is a off groud grid (not reachable forever)
	short travelable;

	//quality computed flag
	bool qualFlag;
	//minimum computed flag
	bool nodeGenFlag;
	//center point of a map grid 
	pcl::PointXYZ oCenterPoint;

	//constructor
	ConfidenceValue() {

		travelTerm = 1.0;//start with 1, which means no need to go there
		boundTerm = 0.0;//start with 1, which means no need to go there
		visiTerm = 0.0;
		qualTerm = 0.0;
		label = 0;//start with nothing
		travelable = -1;//start with unknown
		nodeGenFlag = false;	//start with undone	
		qualFlag = true;//start with undone
		oCenterPoint.x = 0.0;//start from 0, which will be re-define in InitializeGridMap
		oCenterPoint.y = 0.0;//start from 0
		oCenterPoint.z = 0.0;//start from 0

	};

};



class Confidence {

	typedef pcl::PointCloud<pcl::PointXYZ> PCLCloudXYZ;
	typedef pcl::PointCloud<pcl::PointXYZI> PCLCloudXYZI;
	typedef pcl::PointCloud<pcl::PointXYZ>::Ptr PCLCloudXYZPtr;
	typedef pcl::PointCloud<pcl::PointXYZI>::Ptr PCLCloudXYZIPtr;

public:

	//constructor
	Confidence(float f_fSigma,
		       float f_fGHPRParam = 4.2,
		       float f_fVisTermThr = 5);
	
	//destructor
	~Confidence();

	//set the affect radius of Gaussian Kernel
	void SetSigmaValue(const float & f_fSigma);

	//set visibility term related paramters
	void SetVisParas(const float & f_fGHPRParam, const float & f_fVisTermThr);

	//*******************Mathematical Formula Part********************
	
	inline float VectorInnerProduct(const pcl::PointXYZ & oAVec,
		                            const pcl::PointXYZ & oBVec);
	
	//Gaussian Kernel Function
	inline float GaussianKernel(const pcl::PointXYZ & oQueryPo,
		                       const pcl::PointXYZ & oTargerPo,
		                                         float & sigma);
	//linear Kernel Function
	inline float LinearKernel(const float & fTargetVal,
	                        	const float & fThrVal);

	//variance
	inline float StandardDeviation(const PCLCloudXYZ & vCloud);

	//density
	inline float ComputeDensity(const PCLCloudXYZ & vCloud,
								int iSampleTimes = 5,
								bool bKDFlag = true);


	//the 2 norm of a vector
	inline float Compute2Norm(const pcl::PointXYZ & oQueryPo,
		                     const pcl::PointXYZ & oTargerPo);

	//the 1 norm of a vector
	inline float ComputeSquareNorm(const pcl::PointXYZ & oQueryPo,
		                           const pcl::PointXYZ & oTargerPo);

	//compute center
	inline pcl::PointXYZ ComputeCenter(const PCLCloudXYZ & vCloud,
		                       const std::vector<int> & vPointIdx);
	inline pcl::PointXYZ ComputeCenter(const PCLCloudXYZ & vCloud);

	//get random value
	inline std::vector<int> GetRandom(const unsigned int iSize,
		                                 const int iSampleNums);
	//static std::vector<int> GetRandom(const pcl::PointCloud<pcl::PointXYZ>::Ptr & pAllTravelCloud,
	//	                                                                 GridMap & oMaper,
	//	                                                              const int iSampleNums);

	//Compute Euclidean distance
	static float ComputeEuclideanDis(pcl::PointXYZ & oQueryPoint, 
		                            pcl::PointXYZ & oTargetPoint);


	//*******************Feature Term Part********************
	//1. the distance term of confidence map
	//generate the distance feature map of a neighboring grids
	void DistanceTerm(std::vector<ConfidenceValue> & vConfidenceMap,
		                          const pcl::PointXYZ & oRobotPoint,
                           const std::vector<int> & vNearGroundIdxs,
	                               const PCLCloudXYZ & vGroundCloud);


	//2. compute the boundary feature of neighboorhood
	void BoundTerm(std::vector<ConfidenceValue> & vConfidenceMap,
                        const std::vector<int> & vNearGroundIdxs,
	                         const PCLCloudXYZPtr & pGroundCloud,
	                          const PCLCloudXYZPtr & pBoundCloud);

	//3. Compute the occlusion
	/*//0412//void OcclusionTerm(std::vector<ConfidenceValue> & vConfidenceVec,
	                    const std::vector<int> & vNearByIdxs,
		  const std::vector<pcl::PointXYZ> & vHistoryViewPoints,
	                           const PCLCloudXYZ & vTravelCloud,
	     const std::vector<std::vector<int>> & vGridTravelPsIdx,
				             const PCLCloudXYZ & vAllBoundCloud,
		  const std::vector<std::vector<int>> & vGridBoundPsIdx,
	                         const PCLCloudXYZ & vObstacleCloud,
	        const std::vector<std::vector<int>> & vGridObsPsIdx);
	


	//2. quality term of confidence map
	//void QualityTermUsingDensity(std::vector<ConfidenceValue> & vConfidenceVec,
	//	                          const std::vector<int> & vNearByIdxs,
	//	                                 const PCLCloudXYZ & vTravelCloud,
	//               const std::vector<std::vector<int>> & vGridTravelPsIdx,
	//	                               const PCLCloudXYZ & vAllBoundCloud,
	//	            const std::vector<std::vector<int>> & vGridBoundPsIdx,
	//	                               const PCLCloudXYZ & vObstacleCloud,
	//	              const std::vector<std::vector<int>> & vGridObsPsIdx);

	//2. quality term of confidence map
	void QualityTerm(std::vector<ConfidenceValue> & vConfidenceVec,
		                 const std::vector<int> & vNearByIdxs,
		                      const PCLCloudXYZ & vAllBoundCloud,
		   const std::vector<std::vector<int>> & vGridBoundPsIdx,
		                      const PCLCloudXYZ & vObstacleCloud,
		     const std::vector<std::vector<int>> & vGridObsPsIdx);*/


	//4. Compute the boundary item
	//std::vector<float> BoundaryTerm(PCLCloudXYZ & vTravelCloud, PCLCloudXYZ & vBoundCloud, pcl::PointXYZ & oRobotPoint);
	
	//5. Compute the frontier item (constract method)
	//void FrontierTerm(std::vector<ConfidenceValue> & vConfidenceVec, const int & iQueryGrid, const std::vector<int> & vNearByIdxs);

	//6. Compute the total coffidence value
	void ComputeTotalCoffidence(std::vector<ConfidenceValue> & vConfidenceMap, 
	                                                   const int & iQueryIdx);

	//normalization of features
	static void Normalization(std::vector<float> & vFeatures);

	//*******************Public Data Part********************
	pcl::PointXYZ oShowCenter;
	pcl::PointXYZ oShowRobot;

private:

	//sigma parameter of Gaussian function in GaussianKernel()
	float m_fSigma;
	
	//visibility term based paramters
	float m_fGHPRParam;///<parameter of GHPR algorithm
	float m_fVisTermThr;///<the threshold of visibility term

	//weighted of each term for total confidence value
	const float m_fWeightDis;
	const float m_fWeightVis;

	//weighted of each term for total confidence value
	const float m_fLenWeight;
	const float m_fBoundWeight;

	//the searched radius of a query point in density estimation
	const float m_fDensityR;

};



}/*namespace*/


#endif