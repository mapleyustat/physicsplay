cat > t << EOF
RelEMp1-11.pdf
RelEM12-26.pdf
RelEMpp12-24.pdf
pp.24.1-24.4.pdf
RelEMpp25-26.pdf
RelEM27-44.pdf
RelEMpp45-51.pdf
RelEMpp52-56.pdf
RelEMpp52-53.pdf
RelEMp53.1.pdf
RelEMpp54-56.pdf
RelEMpp56.1-73.pdf
RelEMpp74-83.pdf
RelEMpp84-102.pdf
RelEMpp103-113.pdf
RelEMpp114-127.pdf
RelEMpp128-135.pdf
RelEMpp136-146.pdf
RelEMGF1-5.pdf
RelEMpp147-165.pdf
RelEMpp166-180.pdf
RelEMpp166-176.pdf
RelEMpp177.pdf
RelEMpp178.pdf
RelEMpp179-180.pdf
RelEMpp181-195.pdf
450hw1.pdf
PS1soln.pdf
450hw2.pdf
PS2soln.pdf
450hw3.pdf
PS3soln.pdf
450midterm.pdf
450hw4.pdf
PS4soln.pdf
450hw5.pdf
450hw6.pdf
EOF

for i in `cat t` ; do wget http://www.physics.utoronto.ca/~poppitz/epoppitz/PHY450_files/$i ; done