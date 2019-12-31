package com.cosin.executor.trywork;

import java.util.Date;

import com.cosin.executor.trywork.WorkForTry.WorkResult;

/**
 * 周期性执行某项任务，直到任务有结果或者超时的执行器
 * @author Administrator
 *
 */
public class RepeatTryWorkUntilFinishedOrTimeoutExecutor implements Runnable {
	public enum ExecuteResult { // 执行器执行结果
		Success,	// 成功
		Failed,		// 失败
		Timeout		// 超时
	}
	
	private final WorkForTry work;
	private final long timeoutInMill;
	private final long intervalInMillForDoWork;
	
	private ExecuteResult result;

	public RepeatTryWorkUntilFinishedOrTimeoutExecutor(WorkForTry work,
			long timeoutInMill, long intervalInMillForDoWork) {
		super();
		this.work = work;
		this.timeoutInMill = timeoutInMill;
		this.intervalInMillForDoWork = intervalInMillForDoWork;
	}
	
	public ExecuteResult getResult() {
		return result;
	}

	public void run() {
		Date beforeTime = new Date();
		long remainTimeInMill = timeoutInMill; // 剩余时间
		
		while (remainTimeInMill > 0) {
			WorkResult workRes = work.doWork();
			if (WorkResult.Success.equals(workRes)) {
				// 任务执行成功，结束
				this.result = ExecuteResult.Success; 
				return;
			} else if (WorkResult.Failed.equals(workRes)) {
				// 任务执行失败，结束
				this.result = ExecuteResult.Failed;
				return;
			} else {
				// 任务执行结果不明朗，如果还未超时，需要重试，否则按超时处理.
				
				// 等待一个周期
				try {
					Thread.sleep(intervalInMillForDoWork);
				} catch (InterruptedException e) {
					LogHelper.getLogger().error("thread sleep exception.", e);
					Thread.currentThread().interrupt();
				}
				
				Date afterWorkTime = new Date();
				remainTimeInMill = timeoutInMill - 
						(afterWorkTime.getTime() - beforeTime.getTime());
				if (remainTimeInMill <= 0) {
					// 任务执行超时，结束
					work.afterTimeout();
					this.result = ExecuteResult.Timeout;
					return;
				}
			}
		}
	}

}
