# PAL_project
CYCU ICE PAL project 1 and 4 testdata and answer.


在這裡分享自己寫的程式碼是為了讓學弟妹有錢人的程式碼可以學習(也是為了報仇)，希望學弟妹可以從我的程式碼中找到寫完project的靈感。
project4運算邏輯說明:

main():接收input data，決定要接收多少data來運算。
check():負責接token，然後檢查有沒有文法錯誤，最後呼叫pushasm()，將切好的token放入二維陣列以進行後續運算。
caculate():用遞迴的方法做完大部分運算。例1+2, cout<<123....

if、else、while處理:透過pushasm算好的branch target(br)，如同組合語言那般branch到要去的地方，即可實現功能。

function的處理:

先複製一遍原本的function定義，在修改複製後的程式碼
例:
原本的定義

	int double(int x) // by value
	{
		return x*2;
	}
此時呼叫double(2);

先建立 tempvar_0 = 2; (tmepvar的序號每用過一個就+1避免重複)
 
再來將複製的function中的x都替換成tempvar_0,最後在執行複製出來的程式碼

	int double(int tempvar_0) // by value
 	{
  		return tempvar_0*;
	}
另一個例子
	
 	void addone(int &x) // by ref
 	{
  		x++;
	}

假設我呼叫addone(y);
 	
只要將addone中的所有x替換成y即可以完成by ref

	void addone(int &y) // by ref
 	{
  		y++;
	}

想要了解更多可以打開main中的printasm()，會有更詳細的執行細節。

希望學弟妹看到這裡有得到幫助

本人不支持copy別人的程式，但因為我跟我女友被老師誤判為copy，且他不給予我們機測的機會，我認為我蒙受碩大的冤屈，體制內的改革，該講的我都跟賴老師講了，他以沒有時間為由拒絕機測的提議。

我認為我女友的程式之所以跟我的很像，是因為那是我教的，我精心設計多個步驟引導他完成他的project4，如果他真的抄我的，我們不用日夜努力整整一個月，也不可能她的程式碼比我的短幾百行。

我受到下老大的"幹幹幹"鼓舞，我才下定決心教其他人寫程式，但我感受到的是我因為我的熱情被懲罰。

在此，為了表達抗議，我鼓勵真的不想寫程式的學弟妹們，可以將我的程使碼修改到一定程度後繳交，只要他不機測，他就永遠抓不到修飾得當的程式碼。

希望這堂課有天能改成機測為主，程式檢查為輔，老師寧可錯殺不可錯放的態度，真的需要改變。

以下是我粗略想到的對抗方法:

1.改變數名稱(可以用control+f全部一起修改)

2.for改while

3.重組程式碼順序

4.在程式碼中增加很多無用片段，例if(0)、while(false)、cout<<""等

5.呼叫無用function，若怕老師使用組合語言來比對，可以替這些function加上inline修飾詞

6.換一些等價的語法，i++ -> i +=1 ,i-=-1

有問題可以傳訊息到daniel29348679@gmail.com，IG:danieltseng1219

 
 
