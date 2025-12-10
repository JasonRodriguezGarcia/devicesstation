import React, {useState, useEffect} from 'react';
import { useNavigate } from 'react-router-dom';
import { Button, Box, TextField, Typography } from '@mui/material';

function HomePage() {

    const navigate = useNavigate();

    const goToCars = () => {
        navigate("/");
    }

  return (
    <div>
      <h2>Welcome to Home page</h2>

      <Button variant="contained" color="primary" onClick={goToCars}>
        Under develop ...
      </Button>
    </div>
  );
}
export default HomePage;