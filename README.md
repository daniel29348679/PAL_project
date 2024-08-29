# PAL_project
CYCU ICE PAL project 1 and 4 testdata and answer.


在這裡分享自己寫的程式碼是為了讓學弟妹有錢人的程式碼可以學習(也是為了報仇)，希望學弟妹可以從我的程式碼中找到寫完project的靈感。
project4運算邏輯說明:

main():接收input data，決定要接收多少data來運算。
check():負責接token，然後檢查有沒有文法錯誤，最後呼叫pushasm()，將切好的token放入二維陣列以進行後續運算。
caculate():用地回的方法做完大部分運算。

if、else、while處理:透過pushasm算好的branch target(br)，如同組合語言那般branch到要去的地方，即可實現功能。
