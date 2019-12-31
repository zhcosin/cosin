package com.cosin.algorithm.douglaspeuker;

/**
 * 抽稀算法样本
 * @author Administrator
 *
 */
public interface IVacuateSampleData {
	/**
	 * 横坐标，样本点不一定是均匀分布的，所以需要此值
	 * @return
	 */
	double position();
	
	/**
	 * 样本点的值
	 * @return
	 */
	double value();
}
