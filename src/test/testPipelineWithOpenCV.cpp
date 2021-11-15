/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 15:18:18
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-14 22:57:43
 */
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipeNodeDispatcher.hpp"
#include "../core/pipeline/pipeline.hpp"


// #include <opencv2/core/core.hpp> 
// #include <opencv2/highgui/highgui.hpp> 


using namespace vtf::pipeline;

int main()
{

	// Mat M(600,600,CV_8UC1);//创建一个高400，宽200的灰度图的Mat对象
	// namedWindow("Test");     //创建一个名为Test窗口
 
 
	// for (int i=0;i<M.rows;i++)        //遍历每一行每一列并设置其像素值
	// {
	// 	for (int j=0;j<M.cols;j++)
	// 	{
	// 		M.at<uchar>(i,j)=i*j/20;
	// 	}
	// }
 
	// imshow("Test",M);   //窗口中显示图像
	// imwrite("E:/灰度图.jpg",M);    //保存生成的图片
	// waitKey(5000); //等待5000ms后窗口自动关闭
	// getchar();
}