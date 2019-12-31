package com.cosin.executor.trywork;

/**
 * 执行器要周期性执行的任务，任务返回结果：成功、失败、不明确(需要重新再执行以确定结果)
 * @author Administrator
 *
 */
public interface WorkForTry {
	public enum WorkResult {
		Success,	// 成功
		Failed,		// 失败
		Continue,	// 需要再次执行才能确定结果
	}
	/**
	 * 执行任务，并返回任务是否成功，若返回不明确，则执行器在未超时的情况下将周期性的反复执行，直到成功或超时.
	 * 该函数不允许抛出任何异常
	 * @return
	 */
	public WorkResult doWork(); 
	
	/**
	 * 在任务超时后执行的工作
	 */
	public void afterTimeout();
}
