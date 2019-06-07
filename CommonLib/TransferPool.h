#pragma once

namespace CommonLib
{
	namespace Network
	{
		namespace Async
		{
			class Transfer;

			//템플릿 없이 사용할 수 있게 하려고 부득이 인터페이스로 만들었다.
			class ITransferPool
			{
			public:
				virtual Transfer* IdleTransfer()=0;
				virtual void ReturnUsedTransfer(Transfer* transfer)=0;
				virtual int GetPoolSize()=0;
				virtual void Uninitialize()=0;
				virtual void RemakeTransfer(Transfer* transfer) = 0;
			};

			template<typename TransferType>
			class TransferPool : public ITransferPool
			{
			public:
				TransferPool(int numberOfThreads, int poolSize, int idleTime=0):poolSize(poolSize),idleTime(idleTime)
				{
					requester.reset(new Io::Iocp::RequestHandler(numberOfThreads, 0));

					isExit=false;

					for(int i=0; i<poolSize; ++i)
					{
						AddNewTransferToList();
					}
				}

				void AddNewTransferToList()
				{
					idleTransfers.push(new TransferType(*this, *requester));
				}

				~TransferPool()
				{
					//Stop은 Uninitialize()에서 하면서 delete는 왜 소멸자에서 하는가
					//Uninitialize()을 실행해도 일부 스레드에서 transfer인스턴스를 대상으로 작업하고 있을 수 있다.
					for(auto transfer:usedTransfers)
					{
						delete transfer;
					}

					usedTransfers.clear();
				}

				virtual void TransferPool::Uninitialize()
				{
					isExit=true;

					//여기서 Lock를 하면 IdleTransfer()가 NULL을 리턴한다는 보장이 됨
					requester->Stop();

					cs.SafeLock([&]()
					{
						//여기에서 usedTransfers에 있는 모든 TransferType의 Task 취소
						while(!idleTransfers.empty())
						{
							auto transfer = idleTransfers.top();
							idleTransfers.pop();

							_ASSERT(usedTransfers.find(transfer) == usedTransfers.end());

							usedTransfers.insert(transfer);
						}

						_ASSERT(usedTransfers.size() == poolSize);

						for(auto transfer : usedTransfers)
						{
							transfer->Stop();
						}

						requester.reset();
					});
				}

				//연결만 하고 데이터 송수신 안 하는 클라이언트 종료하는 함수
				//작성 하다가 말았다.
				void CloseSleepTransfer()
				{
					_ASSERT(usedTransfers.size()>0);

					Transfer* sleepTransfer=nullptr;

					for(auto transfer:usedTransfers)
					{
						if(transfer->IdleTime()>10)
						{
							if(sleepTransfer==nullptr)
							{
								sleepTransfer=transfer;
							}
							else if(sleepTransfer->IdleTime()<transfer->IdleTime())
							{
								sleepTransfer=transfer;
							}
						}
					}

					if(sleepTransfer)
					{
						Debug::Output(String::Format(L"IdleTransfer Close : %dsec\n",sleepTransfer->IdleTime()));
						sleepTransfer->Reset();
					}
				}

				virtual Transfer* IdleTransfer()
				{
					//조금 길어서 cs가 우려스럽다.
					TransferType* transfer = nullptr;

					cs.SafeLock([&]()
					{
						while(idleTransfers.empty())
						{
							//둘 다 비어있다면 프로세스 종료를 준비중인 것이다.
							if(isExit)
								return;

							cs.Relock(10);
						}

						if(isExit)
							return;

						transfer = idleTransfers.top();
						idleTransfers.pop();

						_ASSERT(usedTransfers.find(transfer) == usedTransfers.end());

						usedTransfers.insert(transfer);
					});

					return transfer;
				}

				virtual void ReturnUsedTransfer(Transfer* transfer)
				{
					cs.SafeLock([&]()
					{
						_ASSERT(usedTransfers.find((TransferType*)transfer) != usedTransfers.end());

						usedTransfers.erase((TransferType*)transfer);

						idleTransfers.push((TransferType*)transfer);
					});
				}

				virtual void RemakeTransfer(Transfer* transfer)
				{
					cs.SafeLock([&]()
					{
						_ASSERT(usedTransfers.find((TransferType*)transfer) != usedTransfers.end());
						
						transfer->Stop();

						usedTransfers.erase((TransferType*)transfer);

						delete (TransferType*)transfer;

						AddNewTransferToList();
					});
				}

				virtual int GetPoolSize()
				{
					return poolSize;
				}

			private:
				set<TransferType*> usedTransfers;
				stack<TransferType*> idleTransfers;

				int poolSize;

				Threading::CriticalSection cs;
				Threading::CriticalSection csSleepTransfer;
				bool isExit;

				std::unique_ptr<Io::Iocp::RequestHandler> requester;

				int idleTime;

			private:
				TransferPool(const TransferPool&); 
				TransferPool& operator=(const TransferPool&); 
			};
		}
	}
}
