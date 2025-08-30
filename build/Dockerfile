FROM wichtounet/budgetwarrior:build

ADD release_debug/gcc-15/bin/server /bin/server
ADD budgetwarrior/tools/yfinance_quote.py /bin/yfinance_quote.py

EXPOSE 8080

CMD ["/bin/server", ""]
