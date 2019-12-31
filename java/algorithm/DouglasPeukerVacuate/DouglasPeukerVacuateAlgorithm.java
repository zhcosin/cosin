package com.cosin.algorithm.douglaspeuker;

import java.util.ArrayList;
import java.util.List;

/**
 * 道格拉斯-普克抽稀算法，是用来对大量冗余的图形数据点进行压缩以提取必要的数据点。
 *该算法实现抽稀的过程是：
 *  1）对曲线的首末点虚连一条直线，求曲线上所有点与直线的距离，并找出最大距离值dmax，用dmax与事先给定的阈值D相比： 
 *  2）若dmax<D，则将这条曲线上的中间点全部舍去；则该直线段作为曲线的近似，该段曲线处理完毕。 
　*    若dmax≥D，保留dmax对应的坐标点，并以该点为界，把曲线分为两部分，对这两部分重复使用该方法，即重复1），2）步，直到所有dmax均<D，即完成对曲线的抽稀。 
 *
 * 显然，本算法的抽稀精度也与阈值相关，阈值越大，简化程度越大，点减少的越多，反之，化简程度越低，点保留的越多，形状也越趋于原曲线。
 * 来源： https://www.xuebuyuan.com/692849.html
 * 
 * @author Administrator
 *
 */
public class DouglasPeukerVacuateAlgorithm {
	/**
	 * 抽稀操作
	 * @param sampleDataSet 样本集，必须按照位置由小到大的顺序，即IsampleData.position()由小到大的顺序.
	 * @param threshord 阀值
	 * @return
	 */
	public static List<IVacuateSampleData> vacuate(List<IVacuateSampleData> sampleDataSet, float threshord) {
		if (sampleDataSet == null || sampleDataSet.isEmpty()) {
			System.err.println("sample data list is empty.");
			return null;
		}
		
		int sampleCount = sampleDataSet.size();
		if (sampleCount < 2) {
			System.err.println(String.format("size of sample data list is %d, can not compute.", sampleCount));
			return null;
		}
		
		if (threshord <= 0) {
			System.err.println(String.format("invalid threshord: %f, it must be a positive real number.", threshord));
			return null;
		}
		
		return vacuateImpl(sampleDataSet, threshord);
	}
	
	
	/**
	 * 算法的递归实现
	 * @param sampleDataSet
	 * @param threshord
	 * @return
	 */
	private static List<IVacuateSampleData> vacuateImpl(List<IVacuateSampleData> sampleDataSet, float threshord) {
		int sampleCount = sampleDataSet.size();
		
		double firstPosition = sampleDataSet.get(0).position();
		double firstValue = sampleDataSet.get(0).value();
		
		double lastPosition = sampleDataSet.get(sampleCount - 1).value();
		double lastValue = sampleDataSet.get(sampleCount - 1).value();
		
		// 与首尾连线距离最远的样本点的下标
		int maxDistanceIndex = 0;
		double maxOffset = 0.0;
		
		for (int i = 1; i < sampleCount - 1; ++i) { // 只遍历除首尾之外的点
			IVacuateSampleData sampleData = sampleDataSet.get(i);
			
			// 计算首尾连线在此处的值.
			double currentPosition = sampleData.position();
			double currentGuessValue = firstValue+(lastValue - firstValue)*(currentPosition - firstPosition)/(lastPosition - firstPosition);
			
			// 如果偏离超过阀值，更新最大偏离点的下标，以及最大偏离值
			double offset = Math.abs(sampleData.value() - currentGuessValue);
			if (offset > maxOffset) {
				maxOffset = offset;
				maxDistanceIndex = i;
			}
		}
		
		// 要被选择的样本点列表
		List<IVacuateSampleData> selectedSampleDataList = new ArrayList<>();
		
		// 如果最大偏离值不超过阀值，则该段折线就由首尾两个点连成的直线代替
		// 否则，将折线从最大偏离点处断开，分成左右两段折线再重复流程.
		if (maxOffset < threshord) {
			selectedSampleDataList.add(sampleDataSet.get(0));
			selectedSampleDataList.add(sampleDataSet.get(sampleCount - 1));
		} else {
			// 从最大偏离点处断开成为两段折线，要注意两段折线共用中间的分界点，因此在合并结果时要防止该点被重复选择.
			List<IVacuateSampleData> sampleListPart1 = sampleDataSet.subList(0, maxDistanceIndex + 1);
			List<IVacuateSampleData> sampleListPart2 = sampleDataSet.subList(maxDistanceIndex, sampleCount);
			
			// 从两段折线中分别进行抽稀的结果
			List<IVacuateSampleData> selectedSampleDataListFromPart1 = vacuateImpl(sampleListPart1, threshord);
			List<IVacuateSampleData> selectedSampleDataListFromPart2 = vacuateImpl(sampleListPart2, threshord);
			
			// 合并结果,但要注意每一段折线抽稀结果的最后一个样本点，跟第二段折线抽稀结果的第一个点是一个点，即两段折线的界点，合并结果时需要避免重复.
			selectedSampleDataList.addAll(selectedSampleDataListFromPart1);
			selectedSampleDataList.addAll(selectedSampleDataListFromPart2.subList(1, selectedSampleDataListFromPart2.size()));
		}
		
		return selectedSampleDataList;
	}
}
