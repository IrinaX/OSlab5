/*Модель задачи синхронизации "Производители-потребители"*/
#include <windows.h>
#include <iostream>
#include "Buffer.h"
#define cProducers 3 /*Количество производителей*/
#define cConsumers 3 /*Количество потребителей*/
#define BufferSize 5 /*Размер буфера*/

int cOperations = 1000; /*Количество операций над буфером*/

HANDLE hSemaphorEmpty;
HANDLE hSemaphorFull;
HANDLE hSemaphorMutex;

DWORD __stdcall getkey(void* b) {
	cin.get();
	return cOperations = 0;
}
/* Изменения в программе должны осуществляться только в главном модуле и только путем добавления кода,
позволяющего синхронизировать действия:
 - внутри "производителей" и "потребителей";
 - в теле главной функции (создание объектов синхронизации и пр.);
 - в глобальном пространстве (объявление переменных и пр.).
*/

/*Исходный вариант потока-производителя*/
DWORD __stdcall producer(void* b) {
	while (cOperations-- > 0) {
		WaitForSingleObject(hSemaphorFull, INFINITE);
		WaitForSingleObject(hSemaphorMutex, INFINITE);
		int item = rand();
		((Buffer*)b)->PutItem(item);
		ReleaseSemaphore(  // освободить семафор и позволить получить доступ к ресурсу другим процессам
			hSemaphorMutex,// дескриптор семафора
			1,             // определяет какое значение должно быть добавлено к счетчику семафора, чаще всего равен единице.
			NULL           // указатель на переменную, куда должно быть перемещено предыдущее значение счетчика.
		);
		ReleaseSemaphore(hSemaphorEmpty, 1, NULL);
		Sleep(500 + rand() % 100);
	}
	return 0;
}

/*Исходный вариант потока-потребителя*/
DWORD __stdcall consumer(void* b) {
	while (cOperations-- > 0) {
		WaitForSingleObject(hSemaphorEmpty, INFINITE);
		WaitForSingleObject(hSemaphorMutex, INFINITE);
		cout << ((Buffer*)b)->GetItem() << endl;
		Sleep(500 + rand() % 100);
		ReleaseSemaphore(hSemaphorMutex, 1, NULL);
		ReleaseSemaphore(hSemaphorFull, 1, NULL);
	}
	return 0;
}


int main()
{
	Buffer* Buf = Buffer::CreateBuffer(5); //Создание буфера
	HANDLE hThreads[cProducers + cConsumers];
	//с помощью двух семафоров организовать потоки
	hSemaphorEmpty = CreateSemaphore(NULL, 0, BufferSize, 0);//пустой буфер
	hSemaphorFull = CreateSemaphore(NULL, BufferSize, BufferSize, 0);//полный буфер
	hSemaphorMutex = CreateSemaphore(NULL, 1, BufferSize, 0);//монопольный доступ
	//
	CreateThread(0, 0, getkey, 0, 0, 0);//Вспомогательный поток, ожидающий нажатие клавиши

	for (int i = 0; i < cProducers; i++)
	{/*Создание потоков-производителей*/
		hThreads[i] = CreateThread(0, 0, producer, Buf, 0, 0);
	}
	for (int i = 0; i < cProducers + cConsumers; i++)
	{/*Создание потоков-потребителей*/
		hThreads[i] = CreateThread(0, 0, consumer, Buf, 0, 0);
	}

	WaitForMultipleObjects(cProducers + cConsumers, hThreads, true, INFINITE);
	CloseHandle(hSemaphorEmpty);
	CloseHandle(hSemaphorFull);
	CloseHandle(hSemaphorMutex);
	cin.get();
	return 0;
}


