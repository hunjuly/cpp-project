#pragma once

namespace CommonLib
{
	namespace Network
	{
		namespace Async
		{
			class Transfer;

			//���ø� ���� ����� �� �ְ� �Ϸ��� �ε��� �������̽��� �������.
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
					//Stop�� Uninitialize()���� �ϸ鼭 delete�� �� �Ҹ��ڿ��� �ϴ°�
					//Uninitialize()�� �����ص� �Ϻ� �����忡�� transfer�ν��Ͻ��� ������� �۾��ϰ� ���� �� �ִ�.
					for(auto transfer:usedTransfers)
					{
						delete transfer;
					}

					usedTransfers.clear();
				}

				virtual void TransferPool::Uninitialize()
				{
					isExit=true;

					//���⼭ Lock�� �ϸ� IdleTransfer()�� NULL�� �����Ѵٴ� ������ ��
					requester->Stop();

					cs.SafeLock([&]()
					{
						//���⿡�� usedTransfers�� �ִ� ��� TransferType�� Task ���
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

				//���Ḹ �ϰ� ������ �ۼ��� �� �ϴ� Ŭ���̾�Ʈ �����ϴ� �Լ�
				//�ۼ� �ϴٰ� ���Ҵ�.
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
					//���� �� cs�� ���������.
					TransferType* transfer = nullptr;

					cs.SafeLock([&]()
					{
						while(idleTransfers.empty())
						{
							//�� �� ����ִٸ� ���μ��� ���Ḧ �غ����� ���̴�.
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
